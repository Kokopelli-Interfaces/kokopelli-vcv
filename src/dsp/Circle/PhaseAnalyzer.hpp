#pragma once

#include <math.h>
#include <cmath>

namespace kokopellivcv {
namespace dsp {

struct PhaseAnalyzer {
  float _last_phase = 0.0f;

  inline PhaseEvent detectEvent(float phase) {
    float phase_change = phase - _last_phase;
    float phase_abs_change = fabs(phase_change);
    bool phase_flip = (phase_abs_change > 0.95 && phase_abs_change <= 1.0);
    PhaseEvent phase_event;
    if (phase_flip && 0 < phase_change) {
      return PhaseEvent::BACKWARD;
    } else if (phase_flip && phase_change < 0) {
      return PhaseEvent::FORWARD;
    } else {
      return PhaseEvent::NONE;
    }
  }
};

} // namespace dsp
} // namespace kokopellivcv
