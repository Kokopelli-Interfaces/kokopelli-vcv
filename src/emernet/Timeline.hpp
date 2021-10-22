#pragma once

#include "Emernet.hpp"
#include <vector>

namespace kokopellivcv {
namespace emernet {

struct Timeline  {
  struct LightMovement {
    Time start;
    Time end;
    PhaseBuffer movement;
  };

  std::vector<LightMovement*> timeline;
};

} // namespace emernet
} // namespace kokopellivcv
