#pragma once

#include "rack.hpp"
#include <math.h>

namespace myrisa {
namespace dsp {

struct PhaseOscillator {
  float phase = 0.0f;
  float freq = 1.0f;

  PhaseOscillator() {}

  inline void setPitch(float pitch) {
    freq = pitch;
  }

  inline void reset(float value) {
    this->phase = std::fmod(value, 1.0f);
  }

  inline void step(float dt) {
    float deltaPhase = freq * dt;
    float sum = phase + deltaPhase;
    this->phase = rack::math::eucMod(sum, 1.0f);
  }

  inline float getPhase() {
    return this->phase;
  }
};

} // namespace dsp
} // namespace myrisa
