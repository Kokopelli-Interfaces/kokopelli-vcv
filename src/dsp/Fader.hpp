#pragma once

#include "dsp/Hearth/definitions.hpp"

namespace kokopellivcv {
namespace dsp {

struct Fader {
  float volume = 1.f;

  // FIXME
  inline void fadeOut() {
    volume = 0.f;
  }

  inline void fadeIn() {
    volume = 1.f;
  }

  inline float step(float signal) {
    return signal * volume;
  }
};


} // namespace dsp
} // namespace kokopellivcv
