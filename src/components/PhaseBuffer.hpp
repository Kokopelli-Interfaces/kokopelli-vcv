#pragma once

#include <math.h>
#include <vector>

#include "Layer.hpp"
#include "../assert.hpp"
#include "Antipop.hpp"
#include "Interpolation.hpp"
#include "rack.hpp"

using namespace std;

namespace myrisa {

// a buffer that can be read and written to via a phase float in [0.0f, 1.0f]
// behaviour undefined if adding or replacing at multiple phase rates
struct PhaseBuffer {
private:
  vector<float> buffer;
  // TODO ???
  AntipopFilter antipop_filter;
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

  int size() {
    return buffer.size();
  }

  void addToBack(float sample) {
    if (size() == 0) {
      buffer.push_back(sample);
      divider.reset();
    } else if (divider.process()) {
      buffer.push_back(sample);
    }
  }

  void addToFront(float sample) {
    if (size() == 0) {
      buffer.insert(buffer.begin(), sample);
      divider.reset();
    } else if (divider.process()) {
      buffer.insert(buffer.begin(), sample);
    }
  }

  void replace(float phase, float sample) {
    ASSERT(0, <, buffer.size());
    ASSERT(0.0f, <=, phase);
    ASSERT(phase, <=, 1.0f);

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

  float getAttenuatedSample(float buffer_sample, float attenuation) {
    ASSERT(0.0f, <=, attenuation);

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

  float read(float phase) {
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

    switch (type) {
    // case Type::AUDIO:
    //   // TODO fix clicks?
    //   return InterpolateHermite(buffer.data(), buffer_position, size);
    // case Type::CV:
    //   return interpolateLinearD(buffer.data(), buffer_position);
    // case Type::PARAM:
    //   return interpolateBSpline(buffer.data(), buffer_position);
    default:
      return buffer[floor(buffer_position)];
    }
  }
};

} // namepsace myrisa
