#pragma once

#include "Song.hpp"
#include "definitions.hpp"

namespace kokopellivcv {
namespace dsp {
namespace circle {

// TODO renam eto grup???
struct Conductor {
  Time start; // FIXME

  char group = 'A';
  int group_movement_n = 1;

  // TODO
  // float freq;

  Movement *group_start_movement = nullptr;
  Movement *prev = nullptr;
  Movement *next = nullptr;

  Conductor() {
    return;
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
