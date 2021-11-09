#pragma once

#include "Song.hpp"
#include "definitions.hpp"
#include "dsp/PhaseAnalyzer.hpp"
#include "dsp/AntipopFilter.hpp"
#include "dsp/Signal.hpp"
#include "Cycle.hpp"
#include "Section.hpp"
#include "rack.hpp"

#include <assert.h>
#include <math.h>
#include <vector>
#include <numeric> // std::iota

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct Engine {
  Song song;
  Inputs inputs;
  Options options;

  // FIXME not set
  kokopellivcv::dsp::SignalType _signal_type = kokopellivcv::dsp::SignalType::AUDIO;

  /* read only */

  Section *_current_section = nullptr;

  LoveDirection _love_direction = LoveDirection::ESTABLISHED;
  bool _cycle_mode = false;

  bool _tune_to_frequency_of_established = true;
  Cycle *_new_cycle = nullptr;

  AntipopFilter _read_antipop_filter;
  AntipopFilter _write_antipop_filter;

  Engine();

  void step();
  float readAll();
  float readEstablished();

  bool isRecording();
  void ascend();
  void descend();
  void progress();
  void regress();

  void toggleTuneToFrequencyOfEstablished();
  void toggleCycleMode();

  int getMostRecentLoopLength();
  float getPhaseOfEstablished();

  void nextSection();
  void deleteCycle(unsigned int cycle_i);
  void nextGroup();

private:
  void nextCycle(CycleEnd cycle_end);
  Cycle* setupNewCycleForRecording();
  inline void write();
  inline void handleBeatChange(PhaseAnalyzer::PhaseEvent flip);
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
