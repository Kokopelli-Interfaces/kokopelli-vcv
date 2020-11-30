#pragma once

#include <math.h>
#include <cmath>

namespace myrisa {
namespace dsp {

/**
   Analyzes a generic periodic signal in [0.0, 1.0].
   Extracts phase flip information, num samples per division, and frequency.
*/
struct PhaseAnalyzer {
  float _last_phase = 0.f;
  bool _direction_forward = true;

  float _last_d_phase = 0.f;

  float _time_since_last_d_phase_zero_crossing = 0.f;
  float _last_d_phase_zero_crossing_phase = 0.f;

  float _period_phase_length = 0.f;
  float _period = 0.f;
  float _division_period = 0.f;

  enum PhaseFlip {
    FORWARD,
    BACKWARD,
    NONE,
  };

  inline float getDivisionPeriod() {
    return _division_period;
  }

  // TODO does not estimate slow signals fast enough
  inline PhaseFlip process(float phase, float sample_time) {
    float d_phase = phase - _last_phase;
    if (d_phase == 0) {
      _time_since_last_d_phase_zero_crossing += sample_time;
      return PhaseFlip::NONE;
    }

    bool d_phase_zero_crossing = std::signbit(_last_d_phase) != std::signbit(d_phase);
    if (d_phase_zero_crossing) {
      _period = _time_since_last_d_phase_zero_crossing;
      _period_phase_length = phase - _last_d_phase_zero_crossing_phase;
      _division_period = _period_phase_length / _period;

      _time_since_last_d_phase_zero_crossing = 0.f;
      _last_d_phase_zero_crossing_phase = phase;
    } else {
      _time_since_last_d_phase_zero_crossing += sample_time;
    }

    _last_phase = phase;
    _last_d_phase = d_phase;

    if (d_phase < -0.95) {
      return PhaseFlip::FORWARD;
    } else if (.95 < d_phase) {
      return PhaseFlip::BACKWARD;
    } else {
      return PhaseFlip::NONE;
    }
  }
};

} // namespace dsp
} // namespace myrisa
