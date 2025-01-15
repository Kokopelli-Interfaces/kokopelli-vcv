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

  int enter_at_movement_i = 0;
  bool skip_in_progression = false;
  bool is_playing = true;

  Time start = 0.f;
  Time period = 0.f;

  Time playhead = 0.f;

  float song_love = 1.f;
  float observer_love = 1.f;

  SignalCapture *signal_capture;
  SignalCapture *love_capture;

  bool loop = false;
  bool trim_beginning_not_end_of_signal = false;


private:
  inline float equalPowerCrossfade(float a, float b, float fade) {
      float fadeA = std::cos(fade * M_PI_2); // Fades out A
      float fadeB = std::sin(fade * M_PI_2); // Fades in B
      return a * fadeA + b * fadeB;
  }

public:

  inline Cycle(Group *immediate_group, int enter_movement_i) {
    this->signal_capture = new SignalCapture(kokopellivcv::dsp::SignalType::AUDIO);
    this->love_capture = new SignalCapture(kokopellivcv::dsp::SignalType::PARAM);
    this->immediate_group = immediate_group;
    this->enter_at_movement_i = enter_movement_i;
  }

  inline ~Cycle() {
    delete signal_capture;
    delete love_capture;
  }

  inline Time getMaxCrossfadeTime() {
    Time max_crossfade_time = signal_capture->_period - this->period;
    if (this->period < max_crossfade_time) {
      max_crossfade_time = this->period;
    }
    return max_crossfade_time;
  }

  inline float niceFade(float signal, FadeTimes fade_times) {
    bool loop_fade = this->period < signal_capture->_period;
    if (loop_fade) {
      Time crossfade_time = fade_times.crossfade;
      Time max_crossfade_time = getMaxCrossfadeTime();
      if (max_crossfade_time < crossfade_time ) {
        crossfade_time = max_crossfade_time;
      }

      if (trim_beginning_not_end_of_signal) {
        if (this->start + this->period - crossfade_time < this->playhead) {
          float crossfade_right_sample = signal_capture->read(this->playhead - this->period);
          float fade = (this->playhead - (this->start + this->period - crossfade_time)) / crossfade_time;
          return equalPowerCrossfade(signal, crossfade_right_sample, fade);
        }
      } else {
        if (this->playhead <= crossfade_time) {
          float crossfade_left_sample = signal_capture->read(this->playhead + this->period);
          float fade = this->playhead / crossfade_time;
          return equalPowerCrossfade(crossfade_left_sample, signal, fade);
        }
      }

      return signal;
    }

    float signal_after_fade_in_out = signal;

    float signal_capture_period = this->signal_capture->_period;
    bool fade_out = signal_capture_period - fade_times.fade_out <= this->playhead;
    if (fade_out) {
      Time fade_out_time = fade_times.fade_out;
      if (signal_capture_period - fade_times.fade_out <= 0.f) {
        fade_out_time = signal_capture_period;
      }

      float crossfade_right_sample = 0.f;
      float fade = (this->playhead - (signal_capture_period - fade_out_time)) / fade_out_time;
      signal_after_fade_in_out = rack::crossfade(signal, crossfade_right_sample, fade);
    }

    bool fade_in = this->playhead <= fade_times.fade_in;
    if (fade_in) {
      Time fade_in_time = fade_times.fade_in;
      if (signal_capture_period  <= fade_times.fade_in) {
        fade_in_time = signal_capture_period;
      }

      float crossfade_left_sample = 0.f;
      float fade = this->playhead / fade_in_time;
      signal_after_fade_in_out = rack::crossfade(crossfade_left_sample, signal_after_fade_in_out, fade);
    }

    return signal_after_fade_in_out;
  }

  inline float readSignal(FadeTimes fade_times) {
    if (signal_capture->_period < this->playhead) {
      return 0.f;
    }

    if (this->song_love == 0.f) {
      return 0.f;
    }

    float signal = signal_capture->read(this->playhead);
    signal = fader.step(this->playhead, signal);
    signal = niceFade(signal, fade_times) * this->song_love;
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

  inline void captureWindowAndAlignPlayhead(Time window) {
    this->period = window;

    if (window < this->signal_capture->_period) {
      trim_beginning_not_end_of_signal = true;
      Time max_crossfade_time = getMaxCrossfadeTime();
      this->signal_capture->fitToWindow(window + max_crossfade_time);
      this->love_capture->fitToWindow(window + max_crossfade_time);

      this->start = max_crossfade_time;
      this->playhead = max_crossfade_time;

      // printf("set cycle start and playhead to %Lf)\n", this->start);
    }

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
