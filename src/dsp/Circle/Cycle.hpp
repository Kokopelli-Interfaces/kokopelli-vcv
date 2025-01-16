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

  Time playhead = 0.f;

  float song_love = 1.f;
  float observer_love = 1.f;

  bool loop = true;

  // read only

  SignalCapture *_signal_capture;
  SignalCapture *_outbound_love_capture;
  Time _start_offset_in_signal = 0.f;
  Time _offset_in_group = 0.f;
  Time _period = 0.f;
  bool _trim_beginning_not_end_of_signal = false;

private:
  inline float equalPowerCrossfade(float a, float b, float fade) {
      float fadeA = std::cos(fade * M_PI_2); // Fades out A
      float fadeB = std::sin(fade * M_PI_2); // Fades in B
      return a * fadeA + b * fadeB;
  }

public:

  inline Cycle(Group *immediate_group, Time group_offset, int enter_movement_i) {
    _signal_capture = new SignalCapture(kokopellivcv::dsp::SignalType::AUDIO);
    _outbound_love_capture = new SignalCapture(kokopellivcv::dsp::SignalType::PARAM);

    this->immediate_group = immediate_group;
    this->enter_at_movement_i = enter_movement_i;

    _offset_in_group = group_offset;
  }

  inline ~Cycle() {
    delete _signal_capture;
    delete _outbound_love_capture;
  }

  inline void setPeriod(Time period) {
    _period = period;
  }

  inline Time getMaxCrossfadeTime() {
    Time max_crossfade_time = _signal_capture->_period - _period;
    if (_period < max_crossfade_time) {
      max_crossfade_time = _period;
    }
    return max_crossfade_time;
  }

  inline float niceFade(float signal, FadeTimes fade_times) {
    bool loop_fade = _period < _signal_capture->_period;
    if (loop_fade) {
      Time crossfade_time = fade_times.crossfade;
      Time max_crossfade_time = getMaxCrossfadeTime();
      if (max_crossfade_time < crossfade_time ) {
        crossfade_time = max_crossfade_time;
      }

      if (_trim_beginning_not_end_of_signal) {
        if (_start_offset_in_signal + _period - crossfade_time < this->playhead) {
          float crossfade_right_sample = _signal_capture->read(this->playhead - _period);
          float fade = (this->playhead - (_start_offset_in_signal + _period - crossfade_time)) / crossfade_time;
          return equalPowerCrossfade(signal, crossfade_right_sample, fade);
        }
      } else {
        if (this->playhead <= crossfade_time) {
          float crossfade_left_sample = _signal_capture->read(this->playhead + _period);
          float fade = this->playhead / crossfade_time;
          return equalPowerCrossfade(crossfade_left_sample, signal, fade);
        }
      }

      return signal;
    }

    float signal_after_fade_in_out = signal;

    float signal_capture_period = _signal_capture->_period;
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
    if (_signal_capture->_period < this->playhead) {
      return 0.f;
    }

    if (this->song_love == 0.f) {
      return 0.f;
    }

    float signal = _signal_capture->read(this->playhead);
    signal = fader.step(this->playhead, signal);
    signal = niceFade(signal, fade_times) * this->song_love;
    return signal;
  }

  inline float readLove() {
    if (_signal_capture->_period < this->playhead) {
      return _outbound_love_capture->read(_outbound_love_capture->_period);
    }

    float love = _outbound_love_capture->read(this->playhead);
    if (1.f < love) {
      return 1.f;
    }
    return love;
  }

  inline void write(float in, float love) {
    _signal_capture->write(playhead, in);
    _outbound_love_capture->write(playhead, love);
  }

  inline void captureWindowAndAlignPlayhead(Time window) {
    _period = window;

    if (window < _signal_capture->_period) {
      _trim_beginning_not_end_of_signal = true;
      Time max_crossfade_time = getMaxCrossfadeTime();
      _signal_capture->fitToWindow(window + max_crossfade_time);
      _outbound_love_capture->fitToWindow(window + max_crossfade_time);

      _start_offset_in_signal = max_crossfade_time;
      this->playhead = max_crossfade_time;

      // printf("set cycle start and playhead to %Lf)\n", _start_offset_in_signal);
    }

    // printf("Cycle End with period %Lf (capture period %Lf)\n", _period, _signal_capture->_period);
  }

  inline void finishWrite() {
    _period = playhead;
    _signal_capture->finishWrite();
    _outbound_love_capture->finishWrite();

    // printf("Cycle End with period %Lf (capture period %Lf)\n", _period, _signal_capture->_period);
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
