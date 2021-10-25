#pragma once

#include "Timeline.hpp"
#include "definitions.hpp"
#include "dsp/PhaseAnalyzer.hpp"
#include "dsp/PhaseOscillator.hpp"
#include "dsp/AntipopFilter.hpp"
#include "dsp/Signal.hpp"
#include "Layer.hpp"
#include "rack.hpp"

#include <assert.h>
#include <math.h>
#include <vector>
#include <numeric> // std::iota

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct Engine {
  bool _use_ext_phase = false;
  float _ext_phase = 0.f;
  float _sample_time = 1.0f;

  // TODO make me an array to support MIX4 & PLAY
  kokopellivcv::dsp::SignalType _signal_type;

  std::vector<unsigned int> _selected_layers_idx;
  std::vector<unsigned int> _saved_selected_layers_idx;
  bool _new_layer_active = true;
  bool _select_new_layers = true;

  unsigned int _active_layer_i;

  bool _layer_mode = false;
  std::vector<unsigned int> _selected_layers_idx_before_layer_mode;
  std::pair<unsigned int, unsigned int> _circle_before_layer_mode;

  /* read only */

  std::pair<unsigned int, unsigned int> _circle = std::make_pair(0, 1);

  Layer *_recording_layer = nullptr;
  RecordParams _record_params;

  PhaseOscillator _phase_oscillator;
  PhaseAnalyzer _phase_analyzer;

  Timeline _timeline;
  TimePosition _timeline_position;

  AntipopFilter _read_antipop_filter;
  AntipopFilter _write_antipop_filter;

  Options _options;

  void step();
  float read();
  float readSelection();
  float readActiveLayer();

  void setCircleToLayer(unsigned int layer_i);
  void skipToActiveLayer();

  void forget();
  bool isRecording();

  void setFixBounds(bool fix_bounds);
  void loop();
  void toggleLayerMode();

  void nextLayer();
  void prevLayer();
  void selectRange(unsigned int layer_i_1, unsigned int layer_i_2);
  void soloSelectLayer(unsigned int layer_i);
  bool isSelected(unsigned int layer_i);
  bool isSolo(unsigned int layer_i);
  void toggleSelectLayer(unsigned int layer_i);
  void toggleSelectActiveLayer();
  void soloOrSelectUpToActiveLayer();
  void deleteLayer(unsigned int layer_i);
  void deleteSelection();

private:
  void fitLayerIntoCircle(Layer* layer);
  void endRecording(bool loop, bool create_new_circle);
  Layer* newRecording();
  inline bool phaseDefined();
  inline void write();
  inline void handleBeatChange(PhaseAnalyzer::PhaseEvent flip);
  inline PhaseAnalyzer::PhaseEvent advanceTimelinePosition();
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
