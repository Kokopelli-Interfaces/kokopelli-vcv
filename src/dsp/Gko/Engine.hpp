#pragma once

#include "Timeline.hpp"
#include "modules/GkoInterface.hpp"
#include "definitions.hpp"
#include "dsp/AntipopFilter.hpp"
#include "dsp/Signal.hpp"
#include "Layer.hpp"
#include "rack.hpp"
#include "Channel.hpp"

#include <assert.h>
#include <math.h>
#include <vector>
#include <string>
#include <numeric> // std::iota
#include <boost/unordered_map.hpp>

namespace myrisa {
namespace dsp {
namespace gko {

struct Engine {



  // TODO make me an array to support MIX4 & PLAY
  // myrisa::dsp::SignalType _signal_type;

  /* read only */


  TimePosition _timeline_position;

  /** IOC components */
  Interface _interface;
  Stepper _stepper;
  Recorder _recorder;
  StateController _state_controller;
  ChannelManager _channel_manager;

  void step();
  float read();

  void selectRange(unsigned int layer_i_1, unsigned int layer_i_2);
  void soloSelectLayer(unsigned int layer_i);
  bool isSelected(unsigned int layer_i);
  void toggleSelectLayer(unsigned int layer_i);

private:
  inline bool phaseDefined();
  inline void write();
  inline void endRecording();
  inline Layer* newRecording();
  inline void handlePhaseEvent(PhaseAnalyzer::PhaseEvent flip);
  inline PhaseAnalyzer::PhaseEvent advanceTimelinePosition();
};

} // namespace gko
} // namespace dsp
} // namespace myrisa
