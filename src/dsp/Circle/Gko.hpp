#pragma once

#include "definitions.hpp"

namespace kokopellivcv {
namespace dsp {
namespace circle {

/** Records Love movement on timeline

    v~~LOVE-G1~~___~~
    ---------------------------------
          ^--LOVE~G2~~~_~

    IDEA: potentialities[timelines]

  This means timeline is handled by external unit
  Members are relative to groups
  Do not need timelinepositions
  AFTERALL, what if you want to go to 2->3->2? it skips back in timeline now! but this is not what we need.
  */
struct Gko {

};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
