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

  rack::dsp::ClockDivider _phase_division_period_calculator_divider;
  float _phase_offset_from_last_calculation = 0.0f;
  float _time_since_last_calculation = 0.f;
  bool _phase_reversed = false;

  enum PhaseFlip {
    FORWARD,
    BACKWARD,
    NONE,
  };

  PhaseAnalyzer() {
    _phase_division_period_calculator_divider.setDivision(2500); // ~1/16 second
  }

  inline float getDivisionPeriod() {
    return _division_period_estimate;
  }

  inline float getSamplesPerBeat(float sample_time) {
    return floor(_division_period_estimate / sample_time);
  }

  inline PhaseFlip process(float phase, float sample_time) {
    float phase_change = phase - _last_phase;
    float phase_abs_change = fabs(phase_change);
    bool phase_flip = (phase_abs_change > 0.95 && phase_abs_change <= 1.0);
    PhaseFlip phase_flip_type;

    if (phase_flip && 0 < phase_change) {
      _phase_offset_from_last_calculation += phase_change - 1;
      _time_since_last_calculation += sample_time;
      phase_flip_type = PhaseFlip::BACKWARD;
    } else if (phase_flip && phase_change < 0) {
      _phase_offset_from_last_calculation += phase_change + 1;
      _time_since_last_calculation += sample_time;
      phase_flip_type = PhaseFlip::FORWARD;
    } else {
      if (!_phase_reversed) {
        _phase_reversed =
          (0 < _phase_offset_from_last_calculation &&
            phase_change < 0) ||
          (_phase_offset_from_last_calculation < 0 &&
            0 < phase_change);
      }

      if (!_phase_reversed) {
        _phase_offset_from_last_calculation += phase_change;
        _time_since_last_calculation += sample_time;
      }

      phase_flip_type = PhaseFlip::NONE;
    }

    if (_phase_division_period_calculator_divider.process()) {
      float abs_change = fabs(_phase_offset_from_last_calculation);
      if (abs_change != 0) {
        _division_period_estimate = _time_since_last_calculation / abs_change;
        // printf("phase period estimate: %fs\n", _phase_period_estimate);
      }

      _phase_reversed = false;
      _phase_offset_from_last_calculation = 0;
      _time_since_last_calculation = 0;
    }

    _last_phase = phase;

    return phase_flip_type;
  }
};

} // namespace dsp
} // namespace myrisa
