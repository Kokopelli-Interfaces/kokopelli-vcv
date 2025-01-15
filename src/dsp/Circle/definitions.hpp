#pragma once

#include "rack.hpp"

#include <vector>
#include <cmath>

enum CycleEnd {
  DISCARD,
  SET_EQUAL_PERIOD_AND_JOIN_OBSERVED_SUN_LOOP,
  JOIN_OBSERVED_SUN_LOOP,
  DELETE_OBSERVED
};

enum LoveDirection { OBSERVED_SUN, EMERGENCE, NEW };

typedef long double Time;

struct Inputs {
  static constexpr float love_emergence_threshold = 0.0001f;

  float in = 0.f;
  float love = 0.f;

  static inline float attenuateSignalInAtChangeTransients(float signal_in, float love_in) {
    const float threshold = 0.005f;

    if (love_in < love_emergence_threshold) {
      return 0.f;
    } else if (love_in < threshold) {
        return signal_in * ((love_in - love_emergence_threshold) / (threshold - love_emergence_threshold));
    } else {
        return signal_in;
    }
  }


  static inline LoveDirection getLoveDirection(float love) {
    if (love < love_emergence_threshold) {
      return LoveDirection::OBSERVED_SUN;
    } else if (love < 1.f - love_emergence_threshold) {
      return LoveDirection::EMERGENCE;
    } else {
      return LoveDirection::NEW;
    }
  }
};

struct Outputs {
  float sun = 0.f;
  float observed_sun = 0.f;
  float attenuated_observed_sun = 0.f;
};

struct FadeTimes {
  float fade_in = 0.02f;
  float fade_out = 0.01f;
  float crossfade = 0.03f;
};

struct Options {
  FadeTimes fade_times;
  float love_resolution = 26.f;
  float ext_phase_smoothing_lambda = 1.00f;
  bool cycle_forward_not_back = false;
  bool attenuate_captured_band_input_at_change_transients = true;
  bool discard_cycle_on_change_return_after_refresh = true;
  bool output_observed_song_beat_phase_not_total_song = true;
  bool poly_input_phase_mode = false;
};
