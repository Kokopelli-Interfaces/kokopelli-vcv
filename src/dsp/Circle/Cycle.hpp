#pragma once

#include "definitions.hpp"
#include "TimeCapture.hpp"
#include "Movement.hpp"
#include "dsp/Signal.hpp"

#include <numeric> // std::iota

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct Cycle {
  Time start;
  Time period;

  Time playhead;

  float love = 1.f;

  Movement movement_at_start;
  Movement* movement;

  TimeCapture *signal_capture;
  TimeCapture *love_capture;

  bool loop = false;

  inline Cycle(Time start, Movement* movement) {
    this->start = start;
    this->signal_capture = new TimeCapture(kokopellivcv::dsp::SignalType::AUDIO);
    this->love_capture = new TimeCapture(kokopellivcv::dsp::SignalType::PARAM);
    this->movement_at_start = *movement;
    this->movement = movement;
  }

  inline ~Cycle() {
    delete signal_capture;
    delete love_capture;
  }

  inline float readSignal() {
    if (this->love == 0.f) {
      return 0.f;
    }

   return signal_capture->read(this->playhead) * this->love;
  }

  inline float readLove() {
    return love_capture->read(this->playhead);
  }

  inline void write(float in, float love) {
    signal_capture->write(playhead, in);
    love_capture->write(playhead, love);
  }

  inline void finishWrite() {
    this->period = playhead;
    printf("Cycle End with period %ld -- %f\n", playhead.tick, playhead.phase);
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
