#pragma once

#include <math.h>
#include <vector>
#include <assert.h>

#include "dsp/Interpolation.hpp"
#include "definitions.hpp"
#include "dsp/Signal.hpp"
#include "rack.hpp"
#include "util/math.hpp"

using namespace kokopellivcv::util;

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct SignalCapture {
  float last_sample = 0.f;
  Time _period = 0.f;

  kokopellivcv::dsp::SignalType _signal_type;
  // std::vector<std::vector<float>> _buffer;

  std::vector<float> _buffer;

  rack::dsp::ClockDivider write_divider;

  SignalCapture(kokopellivcv::dsp::SignalType signal_type) {
    _signal_type = signal_type;
    switch (_signal_type) {
    case kokopellivcv::dsp::SignalType::AUDIO:
      write_divider.setDivision(1);
      // avoid vector resizes when recording so to not have any clicks
      _buffer.reserve(44100 * 360 / 1);
      break;
    case kokopellivcv::dsp::SignalType::CV:
      write_divider.setDivision(10);
      _buffer.reserve(44100 * 360 / 10);
      break;
    case kokopellivcv::dsp::SignalType::GATE: case kokopellivcv::dsp::SignalType::VOCT: case kokopellivcv::dsp::SignalType::VEL:
      write_divider.setDivision(100); // approx every ~.25ms
      _buffer.reserve(44100 * 360 / 100);
      break;
    case kokopellivcv::dsp::SignalType::PARAM:
      write_divider.setDivision(2000); // approx every ~5ms
      _buffer.reserve(44100 * 360 / 2000);
      break;
    }
  }

  inline float read(Time t) {
    if (_period < t) {
      return 0.f;
    }

    long double buffer_position = (t / _period) * _buffer.size();

    if (_signal_type == kokopellivcv::dsp::SignalType::AUDIO) {
      return kokopellivcv::dsp::warpInterpolateHermite(_buffer, buffer_position);
    } else if (_signal_type == kokopellivcv::dsp::SignalType::CV) {
      return kokopellivcv::dsp::warpInterpolateLineard(_buffer, buffer_position);
    } else if (_signal_type == kokopellivcv::dsp::SignalType::PARAM) {
      return kokopellivcv::dsp::warpInterpolateBSpline(_buffer, buffer_position);
    } else {
      return _buffer[floor(buffer_position)];
    }
  }

  inline void fitToWindow(Time window_period) {
    if (_period <= window_period) {
      return;
    }

    Time window_start = _period - window_period;
    long double window_start_buffer_pos = (window_start / _period) * _buffer.size();
    int window_start_i = floor(window_start_buffer_pos);

    _buffer.erase(_buffer.begin(), _buffer.begin()+window_start_i);
    _period = window_period;
    _buffer.resize(_buffer.size());
  }

  inline void finishWrite() {
    _buffer.resize(_buffer.size());
  }

  inline void write(Time t, float sample) {
    if (_period < t) {
      _period = t;
      last_sample = sample;
    }

    // FIXME click upon internal resize
    _buffer.push_back(sample);
  }
};


} // namespace circle
} // namespace dsp
} // namepsace kokopellivcv
