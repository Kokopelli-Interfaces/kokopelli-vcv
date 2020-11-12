#pragma once

#include "rack.hpp"
#include <math.h>

namespace myrisa {

struct LowFrequencyOscillator {
  float phase = 0.0f;
  float freq = 1.0f;

  LowFrequencyOscillator() {}

  void setPitch(float pitch) {
    freq = pitch;
  }

  void reset(float value) {
    this->phase = std::fmod(value, 1.0f);
  }

  void step(float dt) {
    float deltaPhase = freq * dt;
    float sum = phase + deltaPhase;
    this->phase = rack::math::eucMod(sum, 1.0f);
  }

  float getPhase() {
    return this->phase;
  }
};

} // namespace myrisa
