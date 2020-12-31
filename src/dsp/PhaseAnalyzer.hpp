#pragma once

#include <math.h>
#include <cmath>

namespace myrisa {
namespace dsp {

/**
   Tracks a phase signal, typically a saw wave in [0.0, 1.0]. Notifies when signal has flipped, and also estimates the period by sampling the signal and finding the slope, taking into consideration possible phase flips. Works for slow phase signals where one can not wait to get period by waiting for a repetition.
*/
struct PhaseAnalyzer {
  const float _discontinuity_change_threshold = 0.05f;

  float _last_phase = 0.0f;
  float _beat_period_estimate = 1.0f;
  int _beat_n_samples_estimate = 0;

  rack::dsp::ClockDivider _phase_beat_period_calculator_divider;

  float _phase_offset_from_last_calculation = 0.0f;
  float _time_since_last_calculation = 0.f;

  bool _phase_reversed = false;

  enum PhaseEvent {
    FORWARD,
    BACKWARD,
    DISCONTINUITY,
    NONE,
  };

  PhaseAnalyzer() {
    _phase_beat_period_calculator_divider.setDivision(2500); // ~1/16 second
  }

  inline float getBeatPeriod() {
    return _beat_period_estimate;
  }

  inline float getSamplesPerBeat() {
    return _beat_n_samples_estimate;
  }

  inline PhaseEvent process(float phase, float sample_time) {
    float phase_change = phase - _last_phase;
    float phase_abs_change = fabs(phase_change);
    bool phase_flip = (phase_abs_change > 0.95 && phase_abs_change <= 1.0);
    PhaseEvent phase_event;

    if (phase_flip && 0 < phase_change) {
      _phase_offset_from_last_calculation += phase_change - 1;
      _time_since_last_calculation += sample_time;
      phase_event = PhaseEvent::BACKWARD;
    } else if (phase_flip && phase_change < 0) {
      _phase_offset_from_last_calculation += phase_change + 1;
      _time_since_last_calculation += sample_time;
      phase_event = PhaseEvent::FORWARD;
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

      if (_discontinuity_change_threshold < phase_abs_change) {
        phase_event = PhaseEvent::DISCONTINUITY;
      } else {
        phase_event = PhaseEvent::NONE;
      }
    }

    if (_phase_beat_period_calculator_divider.process()) {
      float abs_change = fabs(_phase_offset_from_last_calculation);
      if (abs_change != 0) {
        _beat_period_estimate = _time_since_last_calculation / abs_change;
        _beat_n_samples_estimate = floor(_beat_period_estimate / sample_time);
      }

      _phase_reversed = false;
      _phase_offset_from_last_calculation = 0;
      _time_since_last_calculation = 0;
    }

    _last_phase = phase;

    return phase_event;
  }
};

} // namespace dsp
} // namespace myrisa
