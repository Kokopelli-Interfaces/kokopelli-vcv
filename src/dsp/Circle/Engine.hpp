#pragma once

#include "CircleGroup.hpp"
#include "CircleMember.hpp"
#include "CircleAccessor.hpp"
#include "Gko.hpp"
#include "definitions.hpp"
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
  // read/write
  Inputs _inputs;
  Parameters _params;
  Options _options;

  // read only
  State _state;

  // ioc components
  Gko _gko;
  CircleGroup *_music_circle;

  // misc
  AntipopFilter _read_antipop_filter;
  AntipopFilter _write_antipop_filter;

  void loop();
  void loopLongPress();
  void next();
  void prev();

  float step();

  void startLoving();
  void stopLoving();

  void skipToActiveMember();

  void undo();
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
