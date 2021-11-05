#pragma once

#include "rack.hpp"

#include <vector>

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
  bool use_antipop = false;
  bool bipolar_phase_input = false;
};
