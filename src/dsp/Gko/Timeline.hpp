#pragma once

#include "Layer.hpp"
#include "definitions.hpp"
#include <vector>

namespace myrisa {
namespace dsp {
namespace gko {

/**
   A Timeline is the top level structure for content.
*/
struct Timeline {
  // TODO
  // Layer* rendered_timeline;

  std::vector<Layer*> layers;

  inline float getLayerAttenuation(float time, int layer_i) {
    float attenuation = 0.f;
    for (auto layer : layers) {
      for (auto target_layer_i : layer->target_layers_idx) {
        if (target_layer_i == layer_i) {
          attenuation += layer->readRecordingStrength(time);
          break;
        }
      }

      if (1.f <= attenuation)  {
        return 1.f;
      }
    }

    return attenuation;
  }

  inline float read(float time) {
    // return rendered_timeline->readSignal(time); // TODO
    float signal_out = 0.f;
    for (unsigned int i = 0; i < layers.size(); i++) {
      if (layers[i]->readableAtTime(time)) {
        float attenuation = getLayerAttenuation(time, i);
        signal_out += layers[i]->readSignal(time) * (1.f - attenuation);
      }
    }
    return signal_out;
  }

  inline std::vector<Layer*> getLayersFromIdx(std::vector<int> layer_idx) {
    std::vector<Layer*> selected_layers;
    for (auto layer_id : layer_idx) {
      selected_layers.push_back(layers[layer_id]);
    }
    return selected_layers;
  }

  inline float getLengthOfLayers(std::vector<int> layer_idx) {
    assert(layers.size() != 0);
    assert(layer_idx.size() != 0);
    assert(layer_idx.size() <= layers.size());

    float max_length = 0.f;

    for (auto layer : getLayersFromIdx(layer_idx)) {
      if (max_length < layer->length) {
        max_length = layer->length;
      }
    }

    return max_length;
  }
};

} // namespace frame
} // namespace dsp
} // namespace myrisa
