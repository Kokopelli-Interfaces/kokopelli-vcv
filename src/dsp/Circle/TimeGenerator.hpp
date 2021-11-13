#pragma once

#include "definitions.hpp"
#include "rack.hpp"
#include <math.h>

namespace kokopellivcv {
namespace dsp {
namespace circle {

// TODO set with ext phase
struct TimeGenerator {
  Time _time;

  float _freq = 0.f;
  bool _freq_set = false;

  TimeEvent _last_event = TimeEvent::NONE;

  TimeGenerator() {}

  inline void setTickFrequency(float freq) {
    _freq = freq;
    _freq_set = true;
  }

  inline Time getTime() {
    return _time;
  }

  inline bool isSet() {
    return _freq_set;
  }

  inline float getTickFrequency() {
    return _freq;
  }

  inline void reset(Time time) {
    assert(0.f <= time.phase <= 1.f);
    assert(0 < time.tick);

    _freq_set = false;
    _time = time;
  }

  inline void restart() {
    _time.tick = 0.f;
    _time.phase = 0.f;
  }

  inline TimeEvent getLastTimeEvent() {
    return _last_event;
  }

  inline void clearTimeEvent() {
    _last_event = TimeEvent::NONE;
  }

  inline Time step(float dt) {
    float d_phase = _freq * dt;
    float sum = _time.phase + d_phase;
    if (1.0f < sum) {
      _last_event = TimeEvent::NEXT_TICK;
      _time.tick++;
    }
    _time.phase = rack::math::eucMod(sum, 1.0f);
    return _time;
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
