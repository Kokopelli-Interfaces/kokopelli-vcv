#pragma once

#include "definitions.hpp"
#include "dsp/Signal.hpp"
#include "Village.hpp"
#include "Gko.hpp"

#include "rack.hpp"

#include <assert.h>
#include <math.h>
#include <vector>
#include <numeric> // std::iota

namespace kokopellivcv {
namespace dsp {
namespace hearth {

struct Engine {
  Inputs inputs;
  Options options;

  Village _village;
  Gko _gko;

  // FIXME not set
  kokopellivcv::dsp::SignalType _signal_type = kokopellivcv::dsp::SignalType::AUDIO;

  void step();

  void channelStateReset();
  void voiceObservation();
  void ascend();
  void toggleMovementProgression();
  void cycleForward();
  void cycleBackward();
  void undo();

  void toggleTuneToFrequencyOfObservedSun();

  int getMostRecentVoiceLength();

private:
  inline void write();
};

} // namespace hearth
} // namespace dsp
} // namespace kokopellivcv
