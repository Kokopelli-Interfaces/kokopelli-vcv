#pragma once

#include "definitions.hpp"
#include "Recording.hpp"
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

  float love = 0.f;

  Movement movement_at_start;
  Movement* movement;

  Recording *signal_capture;
  Recording *love_capture;

  bool loop = true;

  inline Cycle(Time start, Movement* movement) {
    this->start = start;
    this->signal_capture = new Recording(kokopellivcv::dsp::SignalType::AUDIO);
    this->love_capture = new Recording(kokopellivcv::dsp::SignalType::PARAM);
    this->movement_at_start = *movement;
    this->movement = movement;
  }

  inline ~Cycle() {
    delete signal_capture;
    delete love_capture;
  }

  inline Time getTimeRelativeToCycleStart(Time song_time) {
    Time cycle_time = song_time;
    cycle_time.tick = song_time.tick - this->movement->start.tick;
    if (this->loop && 0 < cycle_time.tick) {
      cycle_time.tick = cycle_time.tick % period.tick;
    }

    return cycle_time;
  }

  inline float readSignal(Time song_position) {
    if (this->love == 0.f) {
      return 0.f;
    }

    Time cycle_time = getTimeRelativeToCycleStart(song_position);
    if (cycle_time.tick < 0) {
      return 0.f;
    }

   return signal_capture->read(cycle_time) * this->love;
  }

  inline float readLove(Time song_position) {
    if (this->love == 0.f) {
      return 0.f;
    }

    Time cycle_time = getTimeRelativeToCycleStart(song_position);
    if (cycle_time.tick < 0) {
      return 0.f;
    }

    return love_capture->read(cycle_time);
  }

  inline void write(float in, float love) {
    signal_capture->write(playhead, in);
    love_capture->write(playhead, love);
  }

  inline void finishWrite() {
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
