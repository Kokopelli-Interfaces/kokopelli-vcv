#pragma once

#include "rack.hpp"

#include <vector>

// TODO comply
struct Interface {
  float in = 0.f;
  float love = 0.f;
  float ext_phase = 0.f;
  float use_ext_phase = false;
  float sample_time = 1.0f;

  float _lovingActiveThreshold = 0.0001f;

  inline bool isLoving() {
    return _lovingActiveThreshold < love;
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
