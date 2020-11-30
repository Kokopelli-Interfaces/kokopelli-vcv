#pragma once

#include "Layer.hpp"
#include "definitions.hpp"
#include <vector>

namespace myrisa {
namespace dsp {
namespace frame {

  /**
     A Section is a collection of layers.
     Each layer in a section starts at the start of a section.
   */
struct Section {
  // too much cpu usage without rendering, since O(n^2) to compute section out (layers attenuate other layers)
  // TODO multi-dimensional
  // PhaseBuffer *render;
  int n_beats = 1;

  std::vector<Layer*> layers;
};

} // namespace frame
} // namespace dsp
} // namespace myrisa
