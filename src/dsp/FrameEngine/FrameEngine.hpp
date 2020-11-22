#pragma once

#include "dsp/PhaseBuffer.hpp"
#include "definitions.hpp"
#include "dsp/PhaseOscillator.hpp"
#include "rack.hpp"
#include "util/assert.hpp"
#include <assert.h>
#include <math.h>
#include <tuple>
#include <vector>

using namespace std;
using namespace myrisa::util;

namespace myrisa {
namespace dsp {

class FrameEngine {
private:
  class Section;

public:
  bool _use_ext_phase = false;
  float _ext_phase = 0.0f;
  float _in = 0.0f;
  float _sample_time = 1.0f;
  float _delta = 0.0f;
  float _section_position = 0.0f;
  Section *_active_section = nullptr;
  bool _recording = false;

private:
  float _attenuation = 0.0f;

  const int numSections = 16;
  const float record_threshold = 0.05f;

  Section *recording_dest_section = nullptr;

  std::vector<Section*> sections;

public:
  FrameEngine();
  void startRecording();
  Section getActiveSection();
  void endRecording();
  void step();
  float read();
};

class FrameEngine::Section {
private:
  FrameEngine *_engine;

  class Layer;

  vector<Layer*> _layers;
  vector<Layer*> _selected_layers;
  Layer *_active_layer = nullptr;

  PhaseOscillator _phase_oscillator;
  bool _phase_defined = false;

  rack::dsp::ClockDivider _ext_phase_freq_calculator;
  float _freq_calculator_last_capture_phase_distance = 0.0f;

  float _division_time_s = 0.0f;

  float getLayerAttenuation(int layer_i);
  void newLayer(RecordMode layer_mode);
  void advance();

public:
  int _section_division = 0;
  float _phase = 0.0f;
  RecordMode _mode = RecordMode::READ;

  Section(FrameEngine *engine);
  ~Section();

  void setRecordMode(RecordMode new_mode);
  bool isEmpty();
  void undo();
  float read();
  void step();
};

class FrameEngine::Section::Layer {
private:
  PhaseBuffer *buffer;
  PhaseBuffer *send_attenuation;

public:
  RecordMode _mode;
  int start_division = 0;
  int n_divisions = 0;
  int samples_per_division = 0;

  vector<Layer *> target_layers;
  bool fully_attenuated = false;

  Layer(RecordMode record_mode, int division, vector<Layer *> selected_layers,
        int layer_samples_per_division);
  ~Layer();

  void write(int division, float phase, float sample, float attenuation);
  float getBufferPhase(int division, float phase);
  float readSample(int division, float phase);
  float readSampleWithAttenuation(int division, float phase, float attenuation);
  float readSendAttenuation(int division, float phase);
};

} // namespace dsp
} // namespace myrisa
