#pragma once

#include <tuple>
#include <vector>

#include "Layer.hpp"
#include "dsp/PhaseOscillator.hpp"
#include "rack.hpp"
#include <assert.h>
#include "assert.hpp"

using namespace std;

namespace myrisa {

struct Scene {
  enum Mode { DEFINE_DIVISION, DUB, EXTEND, READ };

private:
  vector<Layer*> layers;
  vector<Layer*> selected_layers;

  Layer *new_layer = NULL;

  PhaseOscillator phase_oscillator;
  bool phase_defined = false;
  double last_phase = 0.0f;
  double position = 0.0f;

  float getLayerAttenuation(int layer_i);
  void startNewLayer(Mode layer_mode);
  void finishNewLayer();
  void stepPhase(float sample_time, bool use_ext_phase, float ext_phase);

public:
  virtual ~Scene() {
    for (auto layer : layers) {
      delete layer;
    }

    if (new_layer) {
      delete new_layer;
    }
  }

  double phase = 0.0f;
  Mode mode = Mode::READ;

  void setMode(Mode new_mode);
  bool isEmpty();
  void undo();
  void step(float in, float attenuation_power, float sample_time, bool use_ext_phase, float ext_phase);
  float read();
};
} // namespace myrisa
