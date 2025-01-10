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
  Time _period = 0.f;

  kokopellivcv::dsp::SignalType _signal_type;
  // std::vector<std::vector<float>> _buffer;

  std::vector<float> _buffer;

  SignalCapture(kokopellivcv::dsp::SignalType signal_type) {
    _signal_type = signal_type;
    switch (_signal_type) {
    case kokopellivcv::dsp::SignalType::AUDIO:
      // avoid vector resizes when recording so to not have any clicks (for most recs)
      _buffer.reserve(44100 * 60 / 1);
      break;
    case kokopellivcv::dsp::SignalType::CV:
      _buffer.reserve(44100 * 60 / 10);
      break;
    case kokopellivcv::dsp::SignalType::GATE: case kokopellivcv::dsp::SignalType::VOCT: case kokopellivcv::dsp::SignalType::VEL:
      _buffer.reserve(44100 * 60 / 100);
      break;
    case kokopellivcv::dsp::SignalType::PARAM:
      _buffer.reserve(44100 * 60 / 2000);
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
      // return kokopellivcv::dsp::warpInterpolateHermite(_buffer, buffer_position);
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
    }

    _buffer.push_back(sample);
  }
};


} // namespace circle
} // namespace dsp
} // namepsace kokopellivcv
