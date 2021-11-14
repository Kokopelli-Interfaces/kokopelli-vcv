#pragma once

#include "Cycle.hpp"

#include "definitions.hpp"
#include <vector>

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct Group {
  std::string name = "A";

  Group *group;
  char letter = 'A';
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
