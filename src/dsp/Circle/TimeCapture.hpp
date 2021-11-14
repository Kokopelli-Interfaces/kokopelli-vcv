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
  std::vector<unsigned int> _tick_sample_i;
  unsigned int _current_sample_i = 0;

  rack::dsp::ClockDivider write_divider;

  TimeCapture(kokopellivcv::dsp::SignalType signal_type) {
    _signal_type = signal_type;
    switch (_signal_type) {
    case kokopellivcv::dsp::SignalType::AUDIO:
      write_divider.setDivision(1);
      break;
    case kokopellivcv::dsp::SignalType::CV:
      write_divider.setDivision(10);
      break;
    case kokopellivcv::dsp::SignalType::GATE: case kokopellivcv::dsp::SignalType::VOCT: case kokopellivcv::dsp::SignalType::VEL:
      write_divider.setDivision(100); // approx every ~.25ms
      break;
    case kokopellivcv::dsp::SignalType::PARAM:
      write_divider.setDivision(2000); // approx every ~5ms
      break;
    }
  }

  inline float interpolateBuffer(Time t) {
    // int current_tick_sample_i = _tick_sample_i[t.tick];
    // // FIXME
    // int next_tick_sample_i;
    // if (_tick_sample_i.size() == t.tick) {
    //   int samples_per_tick = _tick_sample_i[1];
    //   next_tick_sample_i = current_tick_sample_i + samples_per_tick;
    // } else {
    //   next_tick_sample_i = _tick_sample_i[t.tick+1];
    // }

    // double phase_offset = (_tick_sample_i[t.tick+1] - _tick_sample_i[t.tick]) * t.phase;
    // double buffer_position = _tick_sample_i[t.tick] + phase_offset;

    long double buffer_position = (t / _period) * _buffer.size();
    return _buffer[floor(buffer_position)];

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

  inline float read(Time t) {
    return interpolateBuffer(t);
  }

  inline void write(Time t, float sample) {
    if (_period < t) {
      _period = t;
    }

    // if (_tick_sample_i.size() == 0) {
    //   assert(t.tick == 0);
    //   _tick_sample_i.push_back(0);
    // } else if (_tick_sample_i.size() <= t.tick) {
    //   _tick_sample_i.resize(t.tick+1);
    //   _tick_sample_i[t.tick] = _current_sample_i;
    // }

    _buffer.push_back(sample);
    // _current_sample_i++;
  }
};


} // namespace circle
} // namespace dsp
} // namepsace kokopellivcv
