#pragma once

#include "definitions.hpp"
#include "SignalCapture.hpp"
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
  float observer_love = 1.f;

  SignalCapture *signal_capture;
  SignalCapture *love_capture;

  bool loop = false;

  // TODO option
  Time max_crossfade_time = 0.02f;
  Time fade_in_time = 0.01f;
  Time fade_out_time = 0.02f;

private:
  inline float equalPowerCrossfade(float a, float b, float fade) {
      float fadeA = std::cos(fade * M_PI_2); // Fades out A
      float fadeB = std::sin(fade * M_PI_2); // Fades in B
      return a * fadeA + b * fadeB;
  }

public:

  inline Cycle(Time start, Group *immediate_group) {
    this->capture_start = start;
    this->signal_capture = new SignalCapture(kokopellivcv::dsp::SignalType::AUDIO);
    this->love_capture = new SignalCapture(kokopellivcv::dsp::SignalType::PARAM);
    this->immediate_group = immediate_group;
  }

  inline ~Cycle() {
    delete signal_capture;
    delete love_capture;
  }

  inline float niceFade(float signal, float fade_time_mult) {
    bool loop_fade = this->period < signal_capture->_period;
    if (loop_fade) {
      Time crossfade_time = signal_capture->_period - this->period;
      float real_max_crossfade_time = max_crossfade_time * fade_time_mult;
      if (real_max_crossfade_time < crossfade_time) {
        crossfade_time = real_max_crossfade_time;
      }

      if (this->playhead <= real_max_crossfade_time) {
        float crossfade_left_sample = signal_capture->read(this->playhead + this->period);
        float fade = this->playhead / real_max_crossfade_time;
        return equalPowerCrossfade(crossfade_left_sample, signal, fade);
      }

      return signal;
    }

    // assert(signal_capture->_period < this->period);
    float real_fade_out_time = fade_out_time * fade_time_mult;
    bool fade_out = this->period - real_fade_out_time <= this->playhead;
    if (fade_out) {
      Time crossfade_start_time = this->period - real_fade_out_time;
      float crossfade_right_sample = 0.f;
      float fade = (this->playhead - crossfade_start_time) / real_fade_out_time;
      return equalPowerCrossfade(signal, crossfade_right_sample, fade);
    }

    float real_fade_in_time = fade_in_time * fade_time_mult;
    bool fade_in = this->playhead <= real_fade_in_time;
    if (fade_in) { // fade in
      float crossfade_left_sample = 0.f;
      float fade = this->playhead / real_fade_in_time;
      return equalPowerCrossfade(crossfade_left_sample, signal, fade);
    }

    return signal;
  }

  inline float readSignal(float fade_time_mult) {
    if (signal_capture->_period < this->playhead) {
      return 0.f;
    }

    if (this->love == 0.f) {
      return 0.f;
    }

    float signal = signal_capture->read(this->playhead);
    signal = niceFade(signal, fade_time_mult) * this->love;
    // return fader.step(signal);
    return signal;
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
    // printf("Cycle End with period %Lf (capture period %Lf)\n", this->period, this->signal_capture->_period);
  }

  inline void finishWrite() {
    this->period = playhead;
    this->signal_capture->finishWrite();
    this->love_capture->finishWrite();

    // printf("Cycle End with period %Lf (capture period %Lf)\n", this->period, this->signal_capture->_period);
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
