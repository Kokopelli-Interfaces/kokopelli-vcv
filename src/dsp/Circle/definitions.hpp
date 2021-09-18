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
  bool next_member = true;
  bool previous_member = true;

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
    if (love <= 0.5) {
      // float engage_attenuation = -1.f * pow((2.f * love - _recordActiveThreshold), 3) + 1.f;
      // float engage_attenuation = -2.f * love + 1.f;
      float engage_attenuation = -1.4144 * pow(love, 0.5f) + 1.f;
      engage_attenuation = rack::clamp(engage_attenuation, 0.f, 1.f);
      return in * (1.f - engage_attenuation);
    }

    return in;
  }
};

struct Options {
  bool use_antipop = false;
  bool strict_recording_lengths = true;
  bool create_new_member_on_reflect = false;
  bool bipolar_phase_input = false;
};
