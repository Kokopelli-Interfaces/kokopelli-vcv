#pragma once

#include "rack.hpp"
#include <math.h>

namespace kokopelli {
namespace dsp {

struct PhaseOscillator {
  float _phase = 0.f;
  float _freq = 0.f;
  bool _freq_set = false;

  PhaseOscillator() {}

  inline void setFrequency(float freq) {
    _freq = freq;
    _freq_set = true;
  }

  inline bool isSet() {
    return _freq_set;
  }

  inline float getFrequency() {
    return _freq;
  }

  inline void reset(float value) {
    _phase = std::fmod(value, 1.0f);
    _freq_set = false;
  }

  inline float step(float dt) {
    float d_phase = _freq * dt;
    float sum = _phase + d_phase;
    _phase = rack::math::eucMod(sum, 1.0f);
    return _phase;
  }

  inline int getSamplesPerPeriod(float sample_time) {
    float period = 1 / _freq;
    return floor(period / sample_time);
  }
};

} // namespace dsp
} // namespace kokopelli
