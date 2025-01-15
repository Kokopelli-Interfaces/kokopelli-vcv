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

  const kokopellivcv::dsp::SignalType _signal_type = kokopellivcv::dsp::SignalType::AUDIO;

  void step();

  bool isRecording();
  void channelStateReset();
  void cycleObservation();
  void ascend();
  void toggleProgression();
  void toggleSkip();

  void cycleForward();

  void nextMovement(bool ignore_skip_flag);
  void prevMovement(bool ignore_skip_flag);
  void goToStartMovement();
  bool isInProgressionMode();

  void deleteCycles();

  int getNumberOfCyclesInGroupAccountingForCurrentMovement();

  int getMostRecentCycleInObservedSongBeat();
  float getPhaseInObservedSong();
  float getBeatPhase();

  int getMostRecentCycleLength(bool in_observed_song);

private:
  inline void write();
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
