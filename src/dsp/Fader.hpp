#pragma once

#include "dsp/Circle/definitions.hpp"

namespace kokopellivcv {
namespace dsp {

struct Fader {
  inline void fadeOut() {
  }

  inline void fadeIn() {
  }

  inline float step(Time time, float signal) {
    return signal;
  }
};


} // namespace dsp
} // namespace kokopellivcv
