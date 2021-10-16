#pragma once

#include "CircleGroup.hpp"
#include "Gko.hpp"
#include "definitions.hpp"
#include "dsp/PhaseAnalyzer.hpp"
#include "dsp/PhaseOscillator.hpp"
#include "dsp/AntipopFilter.hpp"
#include "dsp/Signal.hpp"
#include "rack.hpp"

#include <assert.h>
#include <math.h>
#include <vector>
#include <numeric> // std::iota

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct Engine {
  Parameters* params;

  Gko _gko;
  CircleGroup *_music_circle;
  LoopMode _loop_mode = LoopMode::Group;

  // TODO
  // std::vector<voice_id> active_voices;
  unsigned int _focused_member_i;

  AntipopFilter _read_antipop_filter;
  AntipopFilter _write_antipop_filter;

  Options _options;

  void loop();
  void loopLongPress();
  void next();
  void prev();

  float step();

  void skipToActiveMember();

  void undo();
  bool isRecording();
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
