#pragma once

enum RecordMode { READ, EXTEND, DUB, REPLACE };
enum RecordContext { TIME, SCENE, LAYER };
enum LoopMode { LOOP_TIME, LOOP_SECTION, LOOP_LAYER };

/*
  At each step, what the FrameEngine does to it's collection of layers is a function of these parameters.
 */
struct Delta {
  RecordMode rec_mode = RecordMode::EXTEND;
  RecordContext rec_context = RecordContext::TIME;
  float attenuation = 0.f;
  bool active = false;
};

