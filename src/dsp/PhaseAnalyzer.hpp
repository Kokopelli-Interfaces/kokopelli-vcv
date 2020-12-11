#pragma once

#include <math.h>
#include <cmath>

namespace myrisa {
namespace dsp {

/**
   Tracks a phase signal, typically a saw wave in [0.0, 1.0]. Notifies when signal has flipped, and also estimates the period by sampling the signal and finding the slope, taking into consideration possible phase flips. Works for slow phase signals where one can not wait to get period by waiting for a repetition.
*/
struct PhaseAnalyzer {
  float _last_phase = 0.0f;
  float _division_period_estimate = 1.0f;

  rack::dsp::ClockDivider _slope_estimator_divider;
  float _offset_from_last_sample = 0.0f;
  float _time_from_last_sample = 0.f;
  int _samples_per_division = 1;

  enum PhaseFlip {
    FORWARD,
    BACKWARD,
    NONE,
  };

  PhaseAnalyzer() {
    _slope_estimator_divider.setDivision(2500); // ~1/16 second
  }

  inline float getDivisionPeriod() {
    return _division_period_estimate;
  }

  inline float getSamplesPerDivision() {
    return _samples_per_division;
  }

  inline PhaseFlip process(float phase, float sample_time) {
    float phase_change = phase - _last_phase;
    float phase_abs_change = fabs(phase_change);
    bool phase_flip = (phase_abs_change > 0.95 && phase_abs_change <= 1.0);
    PhaseFlip phase_flip_type;

    if (phase_flip && 0 < phase_change) {
      _offset_from_last_sample += phase_change - 1;
      phase_flip_type = PhaseFlip::BACKWARD;
    } else if (phase_flip && phase_change < 0) {
      _offset_from_last_sample += phase_change + 1;
      phase_flip_type = PhaseFlip::FORWARD;
    } else {
      phase_flip_type = PhaseFlip::NONE;
      bool in_division_phase_flip =
          (0 < _offset_from_last_sample &&
           phase_change < 0) ||
          (_offset_from_last_sample < 0 &&
           0 < phase_change);
      if (!in_division_phase_flip) {
        _offset_from_last_sample += phase_change;
      }
    }

    _time_from_last_sample += sample_time;

    if (_slope_estimator_divider.process()) {
      float abs_change = fabs(_offset_from_last_sample);
      if (abs_change != 0) {
        _division_period_estimate = _time_from_last_sample / abs_change;
        _samples_per_division = floor(_division_period_estimate / sample_time);
        // printf("phase period estimate: %fs\n", _phase_period_estimate);
      }

      _offset_from_last_sample = 0;
      _time_from_last_sample = 0;
    }

    _last_phase = phase;

    return phase_flip_type;
  }
};

} // namespace dsp
} // namespace myrisa
