#pragma once

enum TimeFrame { TIME, SECTION, LAYER };

/**
  At each step, what the Engine does to it's Timeline is a function of these parameters.
*/
struct ManifestParams {
  enum Mode { EXTEND, DUB, REPLACE };

  float in = 0.f;
  Mode mode = Mode::EXTEND;
  TimeFrame time_frame = TimeFrame::TIME;
  float strength = 0.f;
  bool active = false;
};

