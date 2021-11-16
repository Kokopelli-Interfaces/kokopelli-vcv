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

  Movement movement_at_start;
  Movement* movement;

  TimeCapture *signal_capture;
  TimeCapture *love_capture;

  bool loop = false;

  // TODO option
  Time crossfade_time = 0.5f;

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

    if (this->love == 0.f) {
      return 0.f;
    }

    float signal = signal_capture->read(this->playhead);

    if (this->playhead <= crossfade_time) {
      float crossfade_left_sample;
      if (this->playhead + this->period <= signal_capture->_period) {
        crossfade_left_sample = signal_capture->read(this->playhead + this->period);
      } else {
        crossfade_left_sample = signal_capture->last_sample;
      }

      float fade = this->playhead / crossfade_time;
      signal = rack::crossfade(crossfade_left_sample, signal, fade);
    }

    return signal * this->love;
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
