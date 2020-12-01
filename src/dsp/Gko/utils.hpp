#pragma once

#include "Layer.hpp"
#include <math.h>
#include <vector>

namespace myrisa {
namespace dsp {
namespace gko {

inline pair<int, float> splitBeatAndPhase(float pos) {
  double beat;
  double phase = std::modf(pos, &beat);
  return std::pair<int, float>((int)beat, phase);
}

} // namespace frame
} // namespace dsp
} // namespace myrisa
