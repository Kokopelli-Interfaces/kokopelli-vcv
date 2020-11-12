#pragma once

#include <vector>

#include "Layer.hpp"
#include "assert.hpp"
#include "dsp/Antipop.hpp"
#include "dsp/LFO.hpp"
#include "rack.hpp"

using namespace std;

namespace myrisa {

struct Scene {
public:
  enum Mode { ADD, EXTEND, READ };

  float phase = 0.0f;

  Scene::Mode getMode();
  void setMode(Mode new_mode, float sample_time);
  bool isEmpty();
  void undo();
  void step(float in, float attenuation_power, float sample_time);
  void step(float in, float attenuation_power, float sample_time,
            bool use_ext_phase, float ext_phase);
  float read(float sample_time);

private:
  vector<Layer *> layers;
  vector<Layer *> selected_layers;
  Layer *current_layer = NULL;

  int samples_per_division = 0;
  unsigned division = 0;

  bool phase_defined = false;

  LowFrequencyOscillator phase_oscillator;
  float last_phase = 0.0f;

  AntipopFilter antipop_filter;

  Mode mode = Mode::READ;

  void addLayer();
  void removeLayer();
  void endRecording(float sample_time);
  unsigned int getDivisionLength();
  void updateSceneLength();
};

} // namespace myrisa
