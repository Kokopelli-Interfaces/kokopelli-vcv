#pragma once

#include <math.h>
#include <vector>
#include <assert.h>

#include "dsp/Interpolation.hpp"
#include "definitions.hpp"
#include "dsp/Signal.hpp"
#include "rack.hpp"
#include "util/math.hpp"

using namespace std;
using namespace kokopellivcv::util;

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct TimeCapture {
  Time _period;

  kokopellivcv::dsp::SignalType _signal_type;
  // std::vector<std::vector<float>> _buffer;

  std::vector<float> _buffer;

  rack::dsp::ClockDivider write_divider;

  TimeCapture(kokopellivcv::dsp::SignalType signal_type) {
    // avoid vector resizes when recording so to not have any clicks

    _signal_type = signal_type;
    switch (_signal_type) {
    case kokopellivcv::dsp::SignalType::AUDIO:
      write_divider.setDivision(1);
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
    assert(t <= _period);

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

  inline void finishWrite() {
    _buffer.resize(_buffer.size());
  }

  inline void write(Time t, float sample) {
    if (_period < t) {
      _period = t;
    }

    // FIXME click upon internal resize
    _buffer.push_back(sample);
  }
};


} // namespace circle
} // namespace dsp
} // namepsace kokopellivcv