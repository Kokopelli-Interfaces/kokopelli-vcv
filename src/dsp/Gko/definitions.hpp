#pragma once

#include <vector>

enum TimeFrame { TIMELINE, SELECTED_LAYERS, ACTIVE_LAYER };

struct TimelinePosition {
  unsigned int beat = 0;
  double phase = 0.f;
};

/**
  At each step, what the Engine does to it's Timeline is a function of these parameters.
  See the description of Record in the README for behaviour.
*/
struct RecordParams {
  enum Mode {EXTEND, DUB, REPLACE};

  float in = 0.f;
  TimeFrame time_frame = TimeFrame::SELECTED_LAYERS;
  Mode mode = Mode::DUB;
  float strength = 0.f;

  float _recordActiveThreshold = 0.0001f;

  inline bool active() {
    return _recordActiveThreshold < strength;
  }
};
