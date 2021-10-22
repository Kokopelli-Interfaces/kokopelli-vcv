#pragma once

#include "definitions.hpp"
#include "dsp/misc/PhaseBuffer.hpp"
#include "dsp/misc/Signal.hpp"
#include "EmernetNodeGroup.hpp"

namespace kokopellivcv {
namespace emernet {

enum class NodeType { LIVE, CAPTURE };

struct EmernetNode {
  EmernetNodeGroup* _group = nullptr;
  NodeType _type = NodeType::LIVE;

  bool _is_being_loved = false;

  Time _start;
  Time _end;

  // TODO
  // inline void readFromFile
  // inline void saveToFile
  // store hierarchchilay
  // add import option to import arbitrary groups from file

  float getLight(Time play_head);
  float listen(Time play_head);
};

} // namespace emernet
} // namespace kokopellivcv
