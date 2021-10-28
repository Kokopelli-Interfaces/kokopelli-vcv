#pragma once

#include "rack.hpp"

#include <vector>

struct TimePosition {
  unsigned int beat = 0;
  double phase = 0.f;
};

struct RecordParams {
  float in = 0.f;

  float love = 0.f;
  bool fix_bounds = true;

  bool _active = false;
  float _recordActiveThreshold = 0.0001f;

  inline bool active() {
    if (_active && love <= _recordActiveThreshold) {
      _active = false;
    } else if (!_active && _recordActiveThreshold < love) {
      _active = true;
    }

    return _active;
  }

  inline float readIn() {
    // avoids pops when engaging / disengaging love parameter
    if (0.0001f <= love && love <= 0.033f) {
      float engage_attenuation = -1.f * pow((30.f * love - _recordActiveThreshold), 3) + 1.f;
      engage_attenuation = rack::clamp(engage_attenuation, 0.f, 1.f);
      return in * (1.f - engage_attenuation);
    }

    return in;
  }
};

struct Options {
  bool use_antipop = false;
  bool bipolar_phase_input = false;
};
