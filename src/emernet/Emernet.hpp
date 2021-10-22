#pragma once

#include "EmernetNodeGroup.hpp"

#include <assert.h>
#include <math.h>
#include <vector>

namespace kokopellivcv {
namespace emernet {

struct Emernet {
  EmernetNodeGroup* _top;

  std::vector<EmernetNode*> _nodes_with_light;
  std::vector<EmernetNode*> _all_nodes;

  inline float listen() {
  }
};

} // namespace emernet
} // namespace kokopellivcv
