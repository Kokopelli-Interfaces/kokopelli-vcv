#pragma once

enum TimeFrame { TIME, SECTION, LAYER };

/*
  At each step, what the FrameEngine does to it's collection of layers is a function of these parameters.
 */
struct Delta {
  enum Mode { EXTEND, DUB, REPLACE };
  enum Context { TIME, SCENE, LAYER };

  Mode mode = Mode::EXTEND;
  Context context = Context::TIME;
  float power = 0.f;
  bool active = false;
};

