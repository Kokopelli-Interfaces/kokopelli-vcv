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
  std::vector<float> buffer;
  rack::dsp::ClockDivider write_divider;

  Recording(myrisa::dsp::SignalType signal_type) {
    _signal_type = signal_type;
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

  inline void resize(int new_size) {
    int n_samples = new_size / write_divider.getDivision();
    if (n_samples == 0) {
      n_samples++;
    }
    buffer.resize(n_samples);
  }

  inline int size() {
    return buffer.size() * write_divider.getDivision();
  }

  inline void pushBack(float sample) {
    if (size() == 0) {
      write_divider.reset();
      buffer.push_back(sample);
    } else if (write_divider.process()) {
      buffer.push_back(sample);
    }
  }

  inline void write(double phase, float sample) {
    assert(0.f <= phase);
    assert(phase <= 1.0f);
    if (_signal_type == myrisa::dsp::SignalType::PARAM) {
      assert(0.f <= sample);
    }

    if (write_divider.process()) {
      int length = buffer.size();
      float position = length * phase;
      int i = floor(position) == length ? length - 1 : floor(position);

      // TODO different more sophisticated ways to write?
      // FIXME explodes if in oscillator mode
      if (_signal_type == myrisa::dsp::SignalType::AUDIO)  {
        int i2 = ceil(position) == length ? 0 : ceil(position);
        float w = position - i;
        buffer[i] += sample * (1 - w);
        buffer[i2] += sample * (w);
      } else {
        buffer[i] = sample;
      }
    }
  }

  inline float crossfadeSample(float sample, double phase) {
    const int num_audio_samples_to_fade = 50;
    float fade_length = (float)num_audio_samples_to_fade / (float)buffer.size();
    if (1.0f < fade_length) {
      fade_length = 0.5f;
    }

    if (fade_length <= phase) {
      return sample;
    }

    double fade_amount = rescale(phase, 0.0, fade_length, 0.0);
    return rack::crossfade(buffer[buffer.size()-1], sample, fade_amount);
  }

  inline float read(double phase) {
    assert(0.f <= phase);
    assert(phase <= 1.0);

    int size = buffer.size();
    if (size == 0) {
      return 0.f;
    }

    float buffer_position = size * phase;

    int min_samples_for_interpolation = 4;
    if (size < min_samples_for_interpolation) {
      return buffer[floor(buffer_position)];
    }

    if (_signal_type == myrisa::dsp::SignalType::AUDIO) {
      float interpolated_sample = interpolateHermite(buffer.data(), buffer_position, buffer.size());
      return crossfadeSample(interpolated_sample, phase);
    } else if (_signal_type == myrisa::dsp::SignalType::CV) {
      return interpolateLineard(buffer.data(), buffer_position, buffer.size());
    } else if (_signal_type == myrisa::dsp::SignalType::PARAM) {
      return rack::clamp(interpolateBSpline(buffer.data(), buffer_position, buffer.size()), 0.0, 1.0);
    } else {
      return buffer[floor(buffer_position)];
    }
  }
};


} // namespace gko
} // namespace dsp
} // namepsace myrisa
