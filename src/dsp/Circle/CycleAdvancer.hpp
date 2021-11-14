#pragma once

#include "definitions.hpp"
#include "rack.hpp"
#include <math.h>

namespace kokopellivcv {
namespace dsp {
namespace circle {

// TODO set with ext phase
struct CycleAdvancer {
  float _freq = 0.f;
  bool _freq_set = false;
  TimeEvent _last_event = TimeEvent::NONE;

  CycleAdvancer() {}

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

  inline TimeEvent getLastTimeEvent() {
    return _last_event;
  }

  inline void clearTimeEvent() {
    _last_event = TimeEvent::NONE;
  }

  inline void step(Time &time, float dt) {
    float d_phase = _freq * dt;
    float sum = time.phase + d_phase;
    if (1.0f < sum) {
      _last_event = TimeEvent::NEXT_TICK;
      time.tick++;
    }
    time.phase = rack::math::eucMod(sum, 1.0f);
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
