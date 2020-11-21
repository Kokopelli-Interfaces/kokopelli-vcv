#pragma once

#include <math.h>
#include <vector>

#include "Interpolation.hpp"
#include "rack.hpp"
#include "util/assert.hpp"
#include "util/math.hpp"

using namespace std;
using namespace myrisa::util;

namespace myrisa {

// a buffer that can be read and written to via a phase float in [0.0f, 1.0f]
// behaviour undefined if adding or replacing at multiple phase rates
struct PhaseBuffer {
private:
  vector<float> buffer;
  // TODO ???
  rack::dsp::ClockDivider divider;

public:
  enum Type { AUDIO, PARAM, CV, GATE, VOCT, VEL };
  Type type;

  PhaseBuffer(Type type) {
    type = type;
    switch (type) {
    case Type::AUDIO: case Type::CV:
      divider.setDivision(1);
      break;
    case Type::GATE: case Type::VOCT: case Type::VEL:
      divider.setDivision(100); // approx every ~.25ms
      break;
    case Type::PARAM:
      divider.setDivision(2000); // approx every ~5ms
      break;
    }
  }

  inline void resize(int new_size) {
    buffer.resize(new_size / divider.getDivision());
  }

  inline int size() {
    return buffer.size() * divider.getDivision();
  }

  inline void pushBack(float sample) {
    if (size() == 0) {
      divider.reset();
      buffer.push_back(sample);
    } else if (divider.process()) {
      buffer.push_back(sample);
    }
  }

  inline void write(float phase, float sample) {
    ASSERT(0.0f, <=, phase);
    ASSERT(phase, <=, 1.0f);
    if (type == Type::PARAM) {
      ASSERT(0.0f, <=, sample);
    }

    if (divider.process()) {
      int length = buffer.size();
      float position = length * phase;
      int i = floor(position) == length ? length - 1 : floor(position);

      // TODO different more sophisticated ways to write?
      // FIXME explodes if in oscillator mode
      if (type == Type::AUDIO)  {
        int i2 = ceil(position) == length ? 0 : ceil(position);
        float w = position - i;
        buffer[i] += sample * (1 - w);
        buffer[i2] += sample * (w);
      } else {
        buffer[i] = sample;
      }
    }
  }

  inline float getAttenuatedSample(float buffer_sample, float attenuation) {
    float clamped_attenuation = rack::clamp(attenuation, 0.0f, 1.0f);

    switch (type) {
    case Type::GATE:
      if (clamped_attenuation == 1.0f) {
        return 0.0f;
      } else {
        return buffer_sample;
      }
    case Type::VOCT:
      return buffer_sample;
    default:
      return buffer_sample * (1.0f - clamped_attenuation);
    }
  }

  inline float crossfadeSample(float sample, float phase) {
    const int num_audio_samples_to_fade = 50;
    float fade_length = (float)num_audio_samples_to_fade / (float)buffer.size();
    if (1.0f < fade_length) {
      fade_length = 0.5f;
    }

    if (fade_length <= phase) {
      return sample;
    }

    float fade_amount = rescale(phase, 0.0f, fade_length, 0.0f);
    return rack::crossfade(buffer[buffer.size()-1], sample, fade_amount);
  }

  inline float read(float phase) {
    ASSERT(0.0f, <=, phase);
    ASSERT(phase, <=, 1.0f);

    int size = buffer.size();
    if (size == 0) {
      return 0.0f;
    }

    float buffer_position = size * phase;
    int min_samples_for_interpolation = 4;
    if (size < min_samples_for_interpolation) {
      return buffer[floor(buffer_position)];
    }

    if (type == Type::AUDIO) {
      // float interpolated_sample = InterpolateHermite(buffer.data(),
      float interpolated_sample = interpolateBSpline(buffer.data(), buffer_position);
      return crossfadeSample(interpolated_sample, phase);
    } else if (type == Type::CV) {
      return interpolateLinearD(buffer.data(), buffer_position);
    } else if (type == Type::PARAM) {
      return rack::clamp(interpolateBSpline(buffer.data(), buffer_position), 0.0, 1.0);
    } else {
      return buffer[floor(buffer_position)];
    }
  }
};

} // namepsace myrisa
