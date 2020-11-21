#pragma once

#include <tuple>
#include <vector>

#include "util/assert.hpp"
#include "Layer.hpp"
#include "components/PhaseOscillator.hpp"
#include "modules/Frame/interface.hpp"

#include "rack.hpp"
#include <assert.h>

using namespace std;

namespace myrisa {

struct Section {

private:
  vector<Layer*> _layers;
  vector<Layer*> _selected_layers;

  Layer *_active_layer = NULL;

  PhaseOscillator _phase_oscillator;
  bool _phase_defined = false;

  float _sample_time = 1.0f;

  rack::dsp::ClockDivider _ext_phase_freq_calculator;
  float _freq_calculator_last_capture_phase_distance = 0.0f;

  bool _use_ext_phase = false;
  float _ext_phase = 0.0f;
  float _division_time_s = 0.0f;

  float _attenuation = 0.0f;

  inline void newLayer(RecordMode layer_mode);
  inline void startNewLayer();
  inline void finishNewLayer();
  inline float getLayerAttenuation(int layer_i);
  inline void advance();

public:
  Section() {
    _ext_phase_freq_calculator.setDivision(20000);
  }
  virtual ~Section() {
    for (auto layer : _layers) {
      delete layer;
    }

    if (_active_layer) {
      delete _active_layer;
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
