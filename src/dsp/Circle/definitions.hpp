#pragma once

#include "rack.hpp"

#include <vector>

enum TimeEvent {
  NONE,
  NEXT_TICK,
  PREV_TICK,
  DISCONTINUITY
};

enum CycleEnd {
  DISCARD,
  DISCARD_AND_NEXT_MOVEMENT_IN_GROUP,
  DISCARD_AND_NEXT_MOVEMENT,
  JOIN_ESTABLISHED_LOOP,
  JOIN_ESTABLISHED_NO_LOOP,
  JOIN_ESTABLISHED_NO_LOOP_HARD,
  // SET_PERIOD_TO_ESTABLISHED_AND_EMERGE_WITH_MOVEMENT,
  JOIN_ESTABLISHED_AND_CREATE_NEXT_MOVEMENT,
  DO_NOT_LOOP_AND_NEXT_MOVEMENT,
  SET_TO_SONG_PERIOD_AND_NEXT_GROUP
};

// another way to say: ORDER, LIFE, CHAOS
enum LoveDirection { ESTABLISHED, EMERGENCE, NEW };

// typedef long double tIme;

// problem with using just a double: as it increases, the resolution decreases, causing downsampling noises
struct Time {
  long int tick = 0;
  double phase = 0.f;

  float operator/(const Time& other) const {
    return ((double)tick + phase) / ((double)other.tick + other.phase);
  }

  Time operator-(const Time& other) const {
    Time result = *this;
    result.tick -= other.tick;
    result.phase -= other.phase;
    if (result.phase < 0.f) {
      result.tick--;
      result.phase += 1.f;
    }
    return result;
  }

  bool operator<(const Time& other) const {
    if (tick == other.tick) {
      return phase < other.phase;
    }
    return tick < other.tick;
  }

  bool operator>(const Time& other) const {
    if (tick == other.tick) {
      return phase > other.phase;
    }
    return tick > other.tick;
  }
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
