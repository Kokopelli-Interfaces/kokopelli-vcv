#pragma once

#include <tuple>
#include <vector>

#include "../assert.hpp"
#include "Layer.hpp"
#include "PhaseOscillator.hpp"

#include "rack.hpp"
#include <assert.h>

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
  float last_phase = 0.0;
  float position = 0.0;

  inline void startNewLayer(Mode layer_mode);
  inline void finishNewLayer();
  inline float getLayerAttenuation(int layer_i);
  inline void advancePosition(float sample_time, bool use_ext_phase, float ext_phase);

public:
  virtual ~Scene() {
    for (auto layer : layers) {
      delete layer;
    }

    if (new_layer) {
      delete new_layer;
    }
  }

  float phase = 0.0f;
  Mode mode = Mode::READ;

  void setMode(Mode new_mode);
  bool isEmpty();
  void undo();
  float read();
  void step(float in, float attenuation_power, float sample_time, bool use_ext_phase, float ext_phase);
};
} // namespace myrisa
