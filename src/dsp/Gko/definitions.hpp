#pragma once

#include <vector>

enum TimeFrame { SELECTED_LAYERS, TIMELINE, ACTIVE_LAYER };

enum RecordFrame { CIRCLE, TIME, ALT };

struct TimePosition {
  unsigned int beat = 0;
  double phase = 0.f;
};

/**
  At each step, what the Engine does to it's Timeline is a function of these parameters.
  See the description of Record in the README for behaviour.
*/
struct RecordParams {
  enum Mode {DUB, EXTEND, REPLACE};

  float in = 0.f;
  RecordFrame record_frame = RecordFrame::TIME;
  Mode mode = Mode::DUB;
  float strength = 0.f;

  float _recordActiveThreshold = 0.0001f;

  inline bool active() {
    return _recordActiveThreshold < strength;
  }
};

struct Options {
  bool use_antipop = false;
};
