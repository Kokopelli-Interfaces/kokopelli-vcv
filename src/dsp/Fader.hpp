#pragma once

#include "dsp/Circle/definitions.hpp"

namespace kokopellivcv {
namespace dsp {

struct Fader {
  float volume = 1.f;          // Current volume level
  Time playhead_start = 0.f;
  Time fade_duration = 0.f;
  bool fading = false;         // Whether a fade is active

  inline void triggerFadeIn(Time start, Time duration) {
    playhead_start = start;
    fade_duration = duration;
    fading = true;
  }

  // Trigger an instant fade-out
  inline void fadeOut() {
    volume = 0.f;
    fading = false; // Stop any active fading
  }

  // Trigger an instant fade-in
  inline void fadeIn() {
    volume = 1.f;
    fading = false; // Stop any active fading
  }

  // Process a single audio sample
  inline float step(Time playhead, float signal) {
    if (fading) {
        if (playhead < playhead_start || playhead_start + fade_duration < playhead) {
          volume = 1.f;
          fading = false;
        } else {
          Time fade_progress = (playhead - playhead_start) / fade_duration;
          volume = fade_progress;
        }
    }
    return signal * volume;
  }
};

} // namespace dsp
} // namespace kokopellivcv
