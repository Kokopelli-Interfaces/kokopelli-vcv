#pragma once

#include "rack.hpp"

#include <vector>
#include <cmath>

enum CycleEnd {
  DISCARD,
  JOIN_ESTABLISHED_LOOP,
  JOIN_ESTABLISHED_NO_LOOP,
  JOIN_ESTABLISHED_AND_CREATE_SUBGROUP
};

// another way to say: ORDER, LIFE, CHAOS
enum LoveDirection { ESTABLISHED, EMERGENCE, NEW };

typedef long double Time;

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

struct Outputs {
  float sun = 0.f;
  float established = 0.f;
};

struct Options {
  float love_resolution = 1000.f;
};
