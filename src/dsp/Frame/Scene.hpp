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
*/
class Scene {
public:
  bool _internal_phase_defined = false;

  std::vector<Layer *> _layers;
  std::vector<Layer *> _selected_layers;
  Layer *_active_layer = nullptr;

  PhaseOscillator _phase_oscillator;

  float getLayerAttenuation(int layer_i, float current_attenuation);

public:
  int _scene_division = 0;
  float _phase = 0.0f;
  Delta::Mode _mode = Delta::Mode::READ;

  ~Scene();

  bool isEmpty();
  void undo();
  float read(float current_attenuation);
};

} // namespace frame
} // namespace dsp
} // namespace myrisa
