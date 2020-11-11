#pragma once

#include "rack.hpp"
#include <math.h>

// adapted from ZZC's LowFrequencyOscillator
// https://github.com/zezic/ZZC/blob/master/src/shared.hpp

namespace myrisa {

struct LowFrequencyOscillator {
  float phase = 0.0f;
  float lastPhase = 0.0f;
  float freq = 1.0f;

  LowFrequencyOscillator() {}

  void setPitch(float pitch) {
    freq = pitch;
  }

  void reset(float value) {
    this->phase = std::fmod(value, 1.0f);
  }

  bool step(float dt) {
    float deltaPhase = freq * dt;
    float summ = phase + deltaPhase;
    this->lastPhase = this->phase;
    this->phase = rack::math::eucMod(summ, 1.0f);
    bool flipped = freq >= 0.0f ? summ >= 1.0f : summ < 0.0f;
    return flipped;
  }

  float getPhase() {
    return this->phase;
  }
};

} // namespace myrisa
