#pragma once

#include "definitions.hpp"
#include "TimeCapture.hpp"
#include "Movement.hpp"
#include "dsp/Signal.hpp"

#include <numeric> // std::iota

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct Group;

struct Cycle {
  Group* group;

  Time period;

  Time capture_start;

  Time playhead = 0.f;

  float love = 1.f;
  float relative_love = 1.f;

  Movement movement_at_start;
  Movement* movement;

  TimeCapture *signal_capture;
  TimeCapture *love_capture;

  bool loop = false;

  inline Cycle(Time start, Movement* movement, Group *group) {
    this->capture_start = start;
    this->signal_capture = new TimeCapture(kokopellivcv::dsp::SignalType::AUDIO);
    this->love_capture = new TimeCapture(kokopellivcv::dsp::SignalType::PARAM);
    this->movement_at_start = *movement;
    this->movement = movement;
    this->group = group;
  }

  inline ~Cycle() {
    delete signal_capture;
    delete love_capture;
  }

  inline float readSignal() {
    if (signal_capture->_period < this->playhead) {
      return 0.f;
    }

    if (this->love == 0.f || this->relative_love == 0.f) {
      return 0.f;
    }

    return signal_capture->read(this->playhead) * this->love * this->relative_love;
  }

  inline float readLove() {
    if (signal_capture->_period < this->playhead) {
      return love_capture->read(love_capture->_period);
    }

    return love_capture->read(this->playhead);
  }

  inline void write(float in, float love) {
    signal_capture->write(playhead, in);
    love_capture->write(playhead, love);
  }

  inline void finishWrite() {
    this->period = playhead;
    this->signal_capture->finishWrite();
    this->love_capture->finishWrite();
    printf("Cycle End with period %Lf\n", this->period);
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
