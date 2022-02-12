#pragma once

#include "definitions.hpp"
#include "rack.hpp"
#include <math.h>

namespace kokopellivcv {
namespace dsp {
namespace circle {

// TODO set with ext phase
struct TimeAdvancer {
  float _freq = 0.f;
  bool _freq_set = false;

  TimeAdvancer() {}

  inline void setTickFrequency(float freq) {
    _freq = freq;
    _freq_set = true;
  }

  inline bool isSet() {
    return _freq_set;
  }

  inline float getTickFrequency() {
    return _freq;
  }

  inline void step(Time &time, float dt) {
    time += _freq * dt;
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
