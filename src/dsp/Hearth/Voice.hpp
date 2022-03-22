#pragma once

#include "definitions.hpp"
#include "SignalCapture.hpp"
#include "dsp/Signal.hpp"
#include "dsp/Fader.hpp"

namespace kokopellivcv {
namespace dsp {
namespace hearth {

struct Group;

struct Voice {
  VoiceOptions voice_options;

  Group *immediate_group;

  Time period = 0.f;
  Time playhead = 0.f;

  float love = 1.f;
  bool loop = false;

  kokopellivcv::dsp::Fader _fader;

  SignalCapture *_signal_capture;
  SignalCapture *_love_capture;

  inline Voice(Group *immediate_group) {
    _signal_capture = new SignalCapture(kokopellivcv::dsp::SignalType::AUDIO);
    _love_capture = new SignalCapture(kokopellivcv::dsp::SignalType::PARAM);
    this->immediate_group = immediate_group;
  }

  inline ~Voice() {
    delete _signal_capture;
    delete _love_capture;
  }

  inline float niceFade(float signal) {
    bool loop_fade = this->period < _signal_capture->_period;
    if (loop_fade) {
      // return signal;
      Time crossfade_time = _signal_capture->_period - this->period;
      if (this->voice_options.max_crossfade_time < crossfade_time) {
        crossfade_time = this->voice_options.max_crossfade_time;
      }

      if (this->playhead <= crossfade_time) {
        float crossfade_left_sample = _signal_capture->read(this->playhead + this->period);
        float fade = this->playhead / crossfade_time;
        return rack::crossfade(crossfade_left_sample, signal, fade);
      }

      return signal;
    }

    // assert(_signal_capture->_period < this->period);
    bool fade_out = this->period - this->voice_options.fade_out_time <= this->playhead;
    if (fade_out) {
      Time crossfade_start_time = this->period - this->voice_options.fade_out_time;
      float crossfade_right_sample = 0.f;
      float fade = (this->playhead - crossfade_start_time) / this->voice_options.fade_out_time;
      return rack::crossfade(signal, crossfade_right_sample, fade);
    }

    bool fade_in = this->playhead <= this->voice_options.fade_in_time;
    if (fade_in) { // fade in
      float crossfade_left_sample = 0.f;
      float fade = this->playhead / this->voice_options.fade_in_time;
      return rack::crossfade(crossfade_left_sample, signal, fade);
    }

    return signal;
  }

  inline float readSignal() {
    if (_signal_capture->_period < this->playhead) {
      return 0.f;
    }

    if (this->love == 0.f) {
      return 0.f;
    }

    float signal = _signal_capture->read(this->playhead);
    signal = niceFade(signal) * this->love;
    return _fader.step(signal);
  }

  inline float readLove() {
    if (_signal_capture->_period < this->playhead) {
      return _love_capture->read(_love_capture->_period);
    }

    float love = _love_capture->read(this->playhead);
    if (1.f < love) {
      return 1.f;
    }
    return love;
  }

  inline void write(float in, float love) {
    _signal_capture->write(playhead, in);
    _love_capture->write(playhead, love);
  }

  inline void setPeriodToCaptureWindow(Time window) {
    if (window < _signal_capture->_period) {
      Time crossfade_time = _signal_capture->_period - window;
      if (this->voice_options.max_crossfade_time < crossfade_time) {
        crossfade_time = this->voice_options.max_crossfade_time;
      }
      _signal_capture->fitToWindow(window + crossfade_time);
      _love_capture->fitToWindow(window + crossfade_time);
      this->playhead = crossfade_time;
    }

    this->period = window;
    printf("Voice End with period %Lf (capture period %Lf)\n", this->period, _signal_capture->_period);
  }

  inline void finishWrite() {
    this->period = playhead;
    _signal_capture->finishWrite();
    _love_capture->finishWrite();

    printf("Voice End with period %Lf (capture period %Lf)\n", this->period, _signal_capture->_period);
  }
};

} // namespace hearth
} // namespace dsp
} // namespace kokopellivcv
