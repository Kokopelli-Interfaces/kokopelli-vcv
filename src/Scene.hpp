#pragma once

#include <assert.h>
#include <vector>

#include "rack.hpp"
#include "Layer.hpp"
#include "dsp/LFO.hpp"

using namespace std;

namespace myrisa {

struct Scene {
  enum Mode { ADD, EXTEND, READ };

  vector<Layer *> layers;
  vector<Layer *> selected_layers;
  Layer *current_layer = NULL;

  Mode mode = Mode::READ;
  int samples_per_division = 0;

  unsigned division = 0;

  LowFrequencyOscillator phase_oscillator;
  bool ext_phase = false;
  bool ext_phase_flipped = false;
  float last_ext_phase = 0.0f;

  void addLayer();
  void removeLayer();
  void setMode(Mode new_mode, float sample_time);
  void setExtPhase(float ext_phase);
  Mode getMode();
  float getPhase();
  bool stepPhase(float sample_time);
  bool isEmpty();
  unsigned int getDivisionLength();
  void step(float in, float attenuation_power, float sample_time);
  void updateSceneLength();
  float read();
};

} // namespace myrisa
