#pragma once

#include "rack.hpp"

#include <vector>
#include <cmath>

enum VoiceEnd {
  DISCARD,
  NEXT_MOVEMENT_VIA_SHIFT,
  SET_EQUAL_PERIOD_AND_JOIN_OBSERVED_GROUP_LOOP,
  JOIN_OBSERVED_GROUP_LOOP,
  FLOOD
};

enum LoveDirection { OBSERVED_GROUP, EMERGENCE, NEW };

typedef long double Time;

struct Inputs {
  static constexpr float love_emergence_threshold = 0.0001f;

  float in = 0.f;
  float love = 0.f;

  static inline LoveDirection getLoveDirection(float love) {
    if (love < love_emergence_threshold) {
      return LoveDirection::OBSERVED_GROUP;
    } else if (love < 1.f - love_emergence_threshold) {
      return LoveDirection::EMERGENCE;
    } else {
      return LoveDirection::NEW;
    }
  }
};

struct Outputs {
  float sun = 0.f;
  float observed_group = 0.f;
  float attenuated_observed_group = 0.f;
};

struct Options {
  float love_resolution = 1000.f;
  float delay_shiftback = 0.f;
  bool include_moon_in_sun_output = true;
  bool include_unloved_moon_in_sun_output = true;
};

struct VoiceOptions {
  Time max_crossfade_time = 0.05f;
  Time fade_in_time = 0.02f;
  Time fade_out_time = 0.07f;
};
