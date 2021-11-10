#pragma once

#include "rack.hpp"

#include <vector>

enum CycleEnd {
  NO_CYCLE_ENDED,
  DISCARD,
  DISCARD_AND_NEXT_SECTION_IN_GROUP,
  DISCARD_AND_NEXT_SECTION,
  EMERGE_WITH_SECTION,
  // SET_PERIOD_TO_SECTION_AND_EMERGE_WITH_SECTION,
  EMERGE_WITH_SECTION_AND_CREATE_NEXT_SECTION,
  DO_NOT_LOOP_AND_NEXT_SECTION,
  SET_TO_SONG_PERIOD_AND_NEXT_GROUP
};

// another way to say: ORDER, LIFE, CHAOS
enum LoveDirection { ESTABLISHED, EMERGENCE, NEW };

struct TimePosition {
  unsigned int beat = 0;
  double phase = 0.f;
};

struct Inputs {
  static constexpr float love_emergence_threshold = 0.0001f;

  float in = 0.f;
  float love = 0.f;

  static inline LoveDirection getLoveDirection(float love) {
    if (love < love_emergence_threshold) {
      return LoveDirection::ESTABLISHED;
    } else if (love < 1.f - love_emergence_threshold) {
      return LoveDirection::EMERGENCE;
    } else {
      return LoveDirection::NEW;
    }
  }
};

struct Options {
  float love_resolution = 1000.f;
  bool use_antipop = false;
  bool bipolar_phase_input = false;
};
