#pragma once

#include "rack.hpp"

#include <vector>

struct TimePosition {
  unsigned int beat = 0;
  double phase = 0.f;
};

struct RecordParams {
  float in = 0.f;

  float strength = 0.f;
  bool record_on_inner_circle = true;
  bool fix_bounds = true;

  float _recordActiveThreshold = 0.0001f;

  inline bool active() {
    return _recordActiveThreshold < strength;
  }

  inline float readIn() {
    // avoids pops when engaging / disengaging strength parameter
    if (0.0001f <= strength && strength <= 0.033f) {
      float engage_attenuation = -1.f * pow((30.f * strength - _recordActiveThreshold), 3) + 1.f;
      engage_attenuation = rack::clamp(engage_attenuation, 0.f, 1.f);
      return in * (1.f - engage_attenuation);
    }

    return in;
  }
};

struct Options {
  bool use_antipop = false;
  bool strict_recording_lengths = true;
  bool create_new_layer_on_skip_back = false;
  bool bipolar_phase_input = false;
};
