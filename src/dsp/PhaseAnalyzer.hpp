#pragma once

#include "rack.hpp"
#include <math.h>

namespace myrisa {
namespace dsp {

/**
   Tracks a phase signal, typically a saw wave in [0.0, 1.0]. Notifies when signal has flipped, and also estimates the period by sampling the signal and finding the slope, taking into consideration possible phase flips.
*/
struct PhaseAnalyzer {
  float _last_phase = 0.0f;
  float _phase_period_estimate = 1.0f;

  rack::dsp::ClockDivider _slope_estimator_divider;
  float _offset_from_last_sample = 0.0f;

  PhaseAnalyzer() {
    _slope_estimator_divider.setDivision(20000); // ~1/2 second
  }

  inline void process(float phase, float sample_time) {
    float phase_change = phase - _last_phase;
    float phase_abs_change = fabs(phase_change);
    bool phase_flip = (phase_abs_change > 0.95 && phase_abs_change <= 1.0);

    // FIXME has a hard time with internal _scene_division loops
    if (phase_flip && 0 < phase_change) {
      _offset_from_last_sample += phase_change - 1;
    } else if (phase_flip && phase_change < 0) {
      _offset_from_last_sample += phase_change + 1;
    } else {
      bool in_division_phase_flip =
          (0 < _offset_from_last_sample &&
           phase_change < 0) ||
          (_offset_from_last_sample < 0 &&
           0 < phase_change);
      if (!in_division_phase_flip) {
        _offset_from_last_sample += phase_change;
      }
    }

    if (_slope_estimator_divider.process()) {
      float abs_change = fabs(_offset_from_last_sample);
      float phase_change_per_sample = abs_change / _slope_estimator_divider.getDivision();
      if (phase_change_per_sample == 0) {
        _phase_period_estimate = -1;
      } else {
        _phase_period_estimate = sample_time / phase_change_per_sample;
      }

      printf("phase period estimate: %f\n", _phase_period_estimate);

      _offset_from_last_sample = 0;
    }

    _last_phase = phase;
  }
};

} // namespace dsp
} // namespace myrisa
