#pragma once

#include "definitions.hpp"
#include "dsp/Signal.hpp"
#include "Song.hpp"
#include "Gko.hpp"

#include "rack.hpp"

#include <assert.h>
#include <math.h>
#include <vector>
#include <numeric> // std::iota

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct Engine {
  Inputs inputs;
  Options options;

  Song _song;
  Gko _gko;

  // FIXME not set
  kokopellivcv::dsp::SignalType _signal_type = kokopellivcv::dsp::SignalType::AUDIO;

  void step();

  bool isRecording();
  void channelStateReset();
  void cycleObservation();
  void ascend();
  void toggleMovementProgression();
  void cycleForward();
  void cycleBackward();
  void undo();

  void toggleTuneToFrequencyOfObservedSun();

  int getMostRecentCycleLength();

private:
  inline void write();
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
