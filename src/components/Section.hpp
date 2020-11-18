#pragma once

#include <tuple>
#include <vector>

#include "../assert.hpp"
#include "Layer.hpp"
#include "PhaseOscillator.hpp"
#include "Frame_shared.hpp"

#include "rack.hpp"
#include <assert.h>

using namespace std;

namespace myrisa {

struct Section {

private:
  vector<Layer*> layers;
  vector<Layer*> selected_layers;

  Layer *new_layer = NULL;

  PhaseOscillator phase_oscillator;
  bool phase_defined = false;
  float last_phase = 0.0f;
  float last_attenuation = 0.0f;
  float last_sample_time = 1.0f;

  inline void startNewLayer();
  inline void finishNewLayer();
  inline float getLayerAttenuation(int layer_i);
  inline void advance(float sample_time, bool use_ext_phase, float ext_phase);

public:
  virtual ~Section() {
    for (auto layer : layers) {
      delete layer;
    }

    if (new_layer) {
      delete new_layer;
    }
  }

  int division = 0;
  float phase = 0.0f;
  RecordMode mode = RecordMode::READ;

  void setRecordMode(RecordMode new_mode);
  bool isEmpty();
  void undo();
  float read();
  void step(float in, float attenuation_power, float sample_time, bool use_ext_phase, float ext_phase);
};
} // namespace myrisa
