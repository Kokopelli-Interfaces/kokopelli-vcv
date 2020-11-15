#pragma once

#include <math.h>
#include <vector>

#include "../Layer.hpp"
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
  AntipopFilter antipop_filter;
  rack::dsp::ClockDivider divider;
  float last_sample;

public:
  enum BufferType { AUDIO, PARAM, CV, GATE, VOCT, VEL };
  BufferType type;

  PhaseBuffer(BufferType type) {
    type = type;
    switch (type) {
    case BufferType::AUDIO: case BufferType::CV:
      divider.setDivision(1);
      break;
    case BufferType::GATE: case BufferType::VOCT: case BufferType::VEL:
      divider.setDivision(100); // approx every ~.25ms
      break;
    case BufferType::PARAM:
      divider.setDivision(2000); // approx every ~5ms
      break;
    }
  }

  void addToBack(float sample) {
    if (divider.process()) {
      buffer.push_back(sample);
    }
  }

  void addToFront(float sample) {
    if (divider.process()) {
      buffer.insert(sample, buffer.front(), 1);
    }
  }

  void replace(double phase, float sample) {
    ASSERT(0, <, buffer.size());
    ASSERT(0.0f, <=, phase);
    ASSERT(phase, <=, 1.0f);

    if (divider.process()) {
      int length = buffer.size();
      float position = length * phase;
      int i = floor(position) == length ? floor(position) : length - 1;

      // TODO different more sophisticated ways to write?
      // FIXME explodes if in oscillator mode
      if (type == BufferType::AUDIO)  {
        int i2 = ceil(position) == length ? ceil(position) : 0;
        float w = position - i;
        buffer[i] += sample * (1 - w);
        buffer[i2] += sample * (w);
      } else {
        buffer[i] = sample;
      }
    }
  }

  float getAttenuatedBufferSample(double buffer_sample, double attenuation) {
    ASSERT(0.0f, <=, attenuation);

    double clamped_attenuation = rack::clamp(attenuation, 0.0f, 1.0f);

    switch (type) {
    case BufferType::GATE:
      if (clamped_attenuation == 1.0f) {
        return 0.0f;
      } else {
        return buffer_sample;
      }
    case BufferType::VOCT:
      return buffer_sample;
    default:
      return buffer_sample * (1.0f - clamped_attenuation);
    }
  }

  float read(double phase) {
    ASSERT(0.0f, <=, phase);
    ASSERT(phase, <=, 1.0f);

    int size = buffer.size();
    if (size == 0) {
      return 0.0f;
    }

    double buffer_position = size * phase;

    int min_samples_for_interpolation = 4;
    if (size < min_samples_for_interpolation) {
      last_sample = buffer[floor(buffer_position)];
      return last_sample;
    }

    double interpolated_sample;
    switch (type) {
    case BufferType::AUDIO:
      interpolated_sample = InterpolateHermite(buffer.data(), buffer_position, size);
    case BufferType::CV:
      interpolated_sample = interpolateLinearD(buffer.data(), buffer_position);
    case BufferType::PARAM:
      interpolated_sample = interpolateBSpline(buffer.data(), buffer_position);
    case BufferType::GATE: case BufferType::VOCT: case BufferType::VEL:
      interpolated_sample = buffer[floor(buffer_position)];
    }

    last_sample = interpolated_sample;
    return last_sample;
  }
};

} // namepsace myrisa
