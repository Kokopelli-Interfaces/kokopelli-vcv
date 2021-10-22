#pragma once

#include "EmernetNodeGroup.hpp"
#include "LiveNode.hpp"
#include "Gko.hpp"
#include "definitions.hpp"
#include "dsp/AntipopFilter.hpp"
#include "dsp/misc/signal.hpp"
#include "rack.hpp"

#include <assert.h>
#include <math.h>
#include <vector>

namespace kokopellivcv {
namespace emernet {

struct Controller {
  EmernetNodeGroup universe;

  void advance();
  float listen();

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

} // namespace emernet
} // namespace kokopellivcv
