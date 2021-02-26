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

namespace myrisa {
namespace dsp {
namespace gko {

struct Engine {
  bool _use_ext_phase = false;
  float _ext_phase = 0.f;
  float _sample_time = 1.0f;

  // TODO make me an array to support MIX4 & PLAY
  myrisa::dsp::SignalType _signal_type;

  std::vector<unsigned int> _selected_layers_idx;
  std::vector<unsigned int> _saved_selected_layers_idx;
  bool _new_layer_active = true;
  bool _select_new_layers = true;

  unsigned int _active_layer_i;

  /* read only */

  std::pair<unsigned int, unsigned int> _circle = std::make_pair(0, 1);
  unsigned int _loop_length = 1;

  Layer *_recording_layer = nullptr;
  RecordParams _record_params;

  PhaseOscillator _phase_oscillator;
  PhaseAnalyzer _phase_analyzer;

  Timeline _timeline;
  TimePosition _timeline_position;
  ReadTimeFrame _read_time_frame;

  AntipopFilter _read_antipop_filter;
  AntipopFilter _write_antipop_filter;

  Options _options;

  void step();
  float read();
  float readSelection();

  void setCircleToActiveLayer();

  void undo();
  bool isRecording();
  void setRecordMode(RecordParams::Mode mode);
  void setRecordTimeFrame(RecordTimeFrame frame);
  void setReadTimeFrame(ReadTimeFrame frame);

  void selectRange(unsigned int layer_i_1, unsigned int layer_i_2);
  void soloSelectLayer(unsigned int layer_i);
  bool isSelected(unsigned int layer_i);
  bool isSolo(unsigned int layer_i);
  void toggleSelectLayer(unsigned int layer_i);
  void deleteLayer(unsigned int layer_i);
  void deleteSelection();

private:
  void endRecording();
  Layer* newRecording();
  inline bool phaseDefined();
  inline void write();
  inline void handlePhaseEvent(PhaseAnalyzer::PhaseEvent flip);
  inline PhaseAnalyzer::PhaseEvent advanceTimelinePosition();
};

} // namespace gko
} // namespace dsp
} // namespace myrisa
