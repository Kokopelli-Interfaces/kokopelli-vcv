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

  float _period = 0.f;
  float _period_start_phase = 0.f;
  float _period_phase_length = 0.f;

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
      _period += sample_time;
      return PhaseFlip::NONE;
    }

    bool period_end =
      (_last_phase < _period_start_phase && _period_start_phase <= phase) ||
      (phase <= _period_start_phase && _period_start_phase < _last_phase);

    if (period_end) {
      _division_period = _period_phase_length / _period;
      _period = 0.f;
      _period_phase_length = 0.f;
      _period_start_phase = phase;
      printf("---- new period estimate: %fs\n", _division_period);
    } else {
      _period += sample_time;

      if (0.f < d_phase) {
        _period_phase_length += d_phase;
      }
    }

    _last_phase = phase;

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
