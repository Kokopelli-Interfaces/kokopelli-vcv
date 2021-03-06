#pragma once

#include "definitions.hpp"
#include "SignalCapture.hpp"
#include "Movement.hpp"
#include "dsp/Signal.hpp"
#include "dsp/Fader.hpp"

#include <numeric> // std::iota

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct Group;

struct Cycle {
  kokopellivcv::dsp::Fader fader;

  Group *immediate_group;

  bool active = true;

  Time period = 0.f;
  Time capture_start;

  Time playhead = 0.f;

  float love = 1.f;

  Movement movement_at_start;
  Movement* movement;

  SignalCapture *signal_capture;
  SignalCapture *love_capture;

  bool loop = false;

  // TODO option
  Time max_crossfade_time = 0.05f;
  Time fade_in_time = 0.02f;
  Time fade_out_time = 0.07f;

  inline Cycle(Time start, Movement* movement, Group *immediate_group) {
    this->capture_start = start;
    this->signal_capture = new SignalCapture(kokopellivcv::dsp::SignalType::AUDIO);
    this->love_capture = new SignalCapture(kokopellivcv::dsp::SignalType::PARAM);
    this->movement_at_start = *movement;
    this->movement = movement;
    this->immediate_group = immediate_group;
  }

  inline ~Cycle() {
    delete signal_capture;
    delete love_capture;
  }

  inline float niceFade(float signal) {
    bool loop_fade = this->period < signal_capture->_period;
    if (loop_fade) {
      // return signal;
      Time crossfade_time = signal_capture->_period - this->period;
      if (max_crossfade_time < crossfade_time) {
        crossfade_time = max_crossfade_time;
      }

      if (this->playhead <= crossfade_time) {
        float crossfade_left_sample = signal_capture->read(this->playhead + this->period);
        float fade = this->playhead / crossfade_time;
        return rack::crossfade(crossfade_left_sample, signal, fade);
      }

      return signal;
    }

    // assert(signal_capture->_period < this->period);
    bool fade_out = this->period - fade_out_time <= this->playhead;
    if (fade_out) {
      Time crossfade_start_time = this->period - fade_out_time;
      float crossfade_right_sample = 0.f;
      float fade = (this->playhead - crossfade_start_time) / fade_out_time;
      return rack::crossfade(signal, crossfade_right_sample, fade);
    }

    bool fade_in = this->playhead <= fade_in_time;
    if (fade_in) { // fade in
      float crossfade_left_sample = 0.f;
      float fade = this->playhead / fade_in_time;
      return rack::crossfade(crossfade_left_sample, signal, fade);
    }

    return signal;
  }

  inline float readSignal() {
    if (signal_capture->_period < this->playhead) {
      return 0.f;
    }

    if (this->love == 0.f) {
      return 0.f;
    }

    float signal = signal_capture->read(this->playhead);
    signal = niceFade(signal) * this->love;
    return fader.step(signal);
  }

  inline float readLove() {
    if (signal_capture->_period < this->playhead) {
      return love_capture->read(love_capture->_period);
    }

    float love = love_capture->read(this->playhead);
    if (1.f < love) {
      return 1.f;
    }
    return love;
  }

  inline void write(float in, float love) {
    signal_capture->write(playhead, in);
    love_capture->write(playhead, love);
  }

  inline void setPeriodToCaptureWindow(Time window) {
    if (window < this->signal_capture->_period) {
      Time crossfade_time = this->signal_capture->_period - window;
      if (max_crossfade_time < crossfade_time) {
        crossfade_time = max_crossfade_time;
      }
      this->signal_capture->fitToWindow(window + crossfade_time);
      this->love_capture->fitToWindow(window + crossfade_time);
      this->playhead = crossfade_time;
    }

    this->period = window;
    printf("Cycle End with period %Lf (capture period %Lf)\n", this->period, this->signal_capture->_period);
  }

  inline void finishWrite() {
    this->period = playhead;
    this->signal_capture->finishWrite();
    this->love_capture->finishWrite();

    printf("Cycle End with period %Lf (capture period %Lf)\n", this->period, this->signal_capture->_period);
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
