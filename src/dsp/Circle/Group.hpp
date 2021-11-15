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

  std::vector<Cycle*> cycles_in_group;

  inline Time alterPeriodToFitIntoGroup(Time period) {
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
