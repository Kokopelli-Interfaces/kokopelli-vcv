#pragma once

#include "definitions.hpp"
#include "Section.hpp"
#include <vector>

namespace myrisa {
namespace dsp {
namespace frame {

/**
   If the FrameEngine were a music festival, each artist(s) stage is a Scene, and each sound in their performance is a layer.
   When one or more layers are introduced, a new section starts.
   All Scenes are on the same timeline, so even if a Scene is not being read or written, it is still advancing through time.
*/
struct Scene {
  // maps beat #'s to section i's
  std::vector<int> beat_to_section_i;

  std::vector<Section*> sections;
  std::vector<int> section_time_b;

  std::vector<int> _target_layers;
  int _selected_layer;

  int _scene_division = 0;

  // ~Scene();

  // void undo();
  // float read(float current_attenuation);
};

} // namespace frame
} // namespace dsp
} // namespace myrisa
