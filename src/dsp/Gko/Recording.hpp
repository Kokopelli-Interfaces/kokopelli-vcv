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
using namespace myrisa::util;

namespace myrisa {
namespace dsp {
namespace gko {

struct Recording {

  /* read only */

  myrisa::dsp::SignalType _signal_type;
  std::vector<std::vector<float>> _buffer;
  int _samples_per_beat;

  rack::dsp::ClockDivider write_divider;

  Recording(myrisa::dsp::SignalType signal_type, int samples_per_beat) {
    _signal_type = signal_type;
    _samples_per_beat = samples_per_beat;
    switch (_signal_type) {
    case myrisa::dsp::SignalType::AUDIO:
      write_divider.setDivision(1);
      break;
    case myrisa::dsp::SignalType::CV:
      write_divider.setDivision(10);
      break;
    case myrisa::dsp::SignalType::GATE: case myrisa::dsp::SignalType::VOCT: case myrisa::dsp::SignalType::VEL:
      write_divider.setDivision(100); // approx every ~.25ms
      break;
    case myrisa::dsp::SignalType::PARAM:
      write_divider.setDivision(2000); // approx every ~5ms
      break;
    }
  }

  inline void pushBack(float sample) {
    if (_buffer.size() == 0) {
      write_divider.reset();
      _buffer.push_back(std::vector<float>(0));
      _buffer[0].push_back(sample);
    } else if (write_divider.process()) {
      _buffer[0].push_back(sample);
    }
    _samples_per_beat++;
  }

  inline float interpolateBuffer(TimePosition t) {
    assert(t.beat < _buffer.size());

    std::vector<float> nearby;
    double beat_position = _samples_per_beat * t.phase;

    int min_samples_for_interpolation = 4;
    if (_samples_per_beat < min_samples_for_interpolation) {
      return _buffer[t.beat][floor(beat_position)];
    }

    int x1 = (int)floor(beat_position);
    if (x1 == _samples_per_beat) {
      x1--;
    }
    float s1 = _buffer[t.beat][x1];

    float s0;
    if (x1 == 0) {
      if (t.beat == 0) {
        s0 = 0.f;
      } else {
        s0 = _buffer[t.beat - 1][_samples_per_beat - 1];
      }
    } else {
      s0 = _buffer[t.beat][x1 - 1];
    }

    float s2;
    float s3;
    if (x1 == _samples_per_beat - 1) {
      if (t.beat == _buffer.size() - 1) {
        s2 = 0.f;
        s3 = 0.f;
      } else {
        s2 = _buffer[t.beat + 1][0];
        s3 = _buffer[t.beat + 1][1];
      }
    } else {
      s2 = _buffer[t.beat][x1 + 1];
      if (x1 + 1 == _samples_per_beat - 1) {
        if (t.beat == _buffer.size() - 1) {
          s3 = 0.f;
        } else {
          s3 = _buffer[t.beat + 1][0];
        }
      } else {
        s3 = _buffer[t.beat][x1 + 2];
      }
    }

    float sample_phase = rack::math::eucMod(beat_position, 1.0f);
    if (_signal_type == myrisa::dsp::SignalType::AUDIO) {
      return Hermite4pt3oX(s0, s1, s2, s3, sample_phase);
    } else if (_signal_type == myrisa::dsp::SignalType::CV) {
      return rack::crossfade(s1, s2, sample_phase);
    } else if (_signal_type == myrisa::dsp::SignalType::PARAM) {
      return rack::clamp(BSpline(s0, s1, s2, s3, sample_phase), 0.f, 10.f);
    } else {
      return s1;
    }
  }

  inline void write(TimePosition t, float sample) {
    assert(0.f <= t.phase);
    assert(t.phase <= 1.0f);

    unsigned int n_beats = _buffer.size();
    while (n_beats <= t.beat) {
      _buffer.push_back(std::vector<float>(_samples_per_beat));
      n_beats++;
    }

    double beat_position = _samples_per_beat * t.phase;
    int x1 = (int)floor(beat_position);
    if (x1 == _samples_per_beat) {
      x1--;
    }

    if (_signal_type == myrisa::dsp::SignalType::AUDIO)  {
      float sample_phase = beat_position - x1;
      _buffer[t.beat][x1] = rack::crossfade(_buffer[t.beat][x1], sample, sample_phase);

      int x2 = ceil(beat_position);
      if (ceil(beat_position) == _samples_per_beat) {
        if (t.beat != _buffer.size() - 1) {
          _buffer[t.beat + 1][0] = rack::crossfade(_buffer[t.beat + 1][0], sample, sample_phase);
        }
      } else {
        _buffer[t.beat][x2] = rack::crossfade(sample, _buffer[t.beat][x2], sample_phase);
      }
    } else {
      _buffer[t.beat][x1] = sample;
    }
  }

  inline float read(TimePosition t) {
    int size = _buffer.size();
    if (size == 0) {
      return 0.f;
    }

    return interpolateBuffer(t);
  }
};


} // namespace gko
} // namespace dsp
} // namepsace myrisa
