#include "Engine.hpp"

using namespace myrisa::dsp::gko;
using namespace myrisa::dsp;

// TODO consider layer attenuations, so that you can't select fully attenuated layer

void Engine::selectRange(unsigned int layer_i_1, unsigned int layer_i_2) {
  unsigned int i1 = layer_i_1;
  unsigned int i2 = layer_i_2;
  if (i2 < i1) {
    i2 = i1;
    i1 = layer_i_2;
  }

  std::vector<unsigned int> selected_layers_idx;
  for (unsigned int i = i1; i <= i2; i++) {
    selected_layers_idx.push_back(i);
  }
  _selected_layers_idx = selected_layers_idx;
}

void Engine::soloSelectLayer(unsigned int layer_i) {
  _selected_layers_idx.erase(_selected_layers_idx.begin(), _selected_layers_idx.end());
  _selected_layers_idx.push_back(layer_i);
}

bool Engine::isSelected(unsigned int layer_i) {
  for (auto layer_i : _selected_layers_idx) {
    if (layer_i == _active_layer_i) {
      return true;
    }
  }
  return false;
}

void Engine::toggleSelectLayer(unsigned int layer_i) {
    bool select = true;
    for (unsigned int layer_i_i = 0; layer_i_i < _selected_layers_idx.size(); layer_i_i++) {
      if (_selected_layers_idx[layer_i_i] == _active_layer_i) {
        _selected_layers_idx.erase(_selected_layers_idx.begin() + layer_i_i);
        select = false;
      }
    }
    if (select && _timeline.layers.size() != 0) {
      _selected_layers_idx.push_back(_active_layer_i);
    }
}
