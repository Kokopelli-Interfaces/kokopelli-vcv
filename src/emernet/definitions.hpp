#pragma once

#include "rack.hpp"
#include "dsp/misc/Signal.hpp"
#include "EmernetNodeGroup.hpp"

#include <vector>

namespace kokopellivcv {
namespace emernet {

typedef std::vector<unsigned int> emernet_node_id;
enum class LoopMode { None, Group, Member };

struct Time {
  unsigned int beat = 0;
  double phase = 0.f;
};

} // namespace emernet
} // namespace kokopellivcv
