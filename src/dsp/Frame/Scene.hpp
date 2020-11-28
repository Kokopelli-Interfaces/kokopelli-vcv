#pragma once

#include "Layer.hpp"
#include "definitions.hpp"
#include "dsp/PhaseOscillator.hpp"
#include <vector>

namespace myrisa {
namespace dsp {
namespace frame {

/**
   A Scene is a collection of layers.
   If the FrameEngine were a music festival, each artist(s) stage is a Scene, and each sound in their performance is a layer.
   All Scenes are on the same timeline, so even if a Scene is not being read or written, it is still advancing through time.
*/
class Scene {
public:
  // TODO ???
  // std::vector<Section *> sections;

  bool _internal_phase_defined = false;

  std::vector<Layer *> _layers;
  std::vector<Layer *> _selected_layers;
  Layer *_active_layer = nullptr;


  float getLayerAttenuation(int layer_i, float current_attenuation);

public:
  int _scene_division = 0;
  float _phase = 0.0f;

  ~Scene();

  bool isEmpty();
  void undo();
  float read(float current_attenuation);
};

} // namespace frame
} // namespace dsp
} // namespace myrisa
