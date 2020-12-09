#pragma once

#include "Layer.hpp"
#include "definitions.hpp"
#include <vector>

namespace myrisa {
namespace dsp {
namespace gko {

/**
   The Timeline is the top level structure for content.
*/
struct Timeline {
  std::vector<Layer*> layers;

  std::vector<float> current_attenuation;
  std::vector<float> last_calculated_attenuation;
  rack::dsp::ClockDivider attenuation_calculator_divider;

  Timeline() {
    attenuation_calculator_divider.setDivision(2000);
  }

  static inline float smoothValue(float current, float old) {
    const float lambda = 30.f / 44100;
    return old + (current - old) * lambda;
  }

  inline void updateLayerAttenuations(TimelinePosition position) {
    if (attenuation_calculator_divider.process()) {
      for (unsigned int layer_i = 0; layer_i < layers.size(); layer_i++) {
      float layer_i_attenuation = 0.f;
        for (unsigned int j = layer_i; j < layers.size(); j++) {
          for (auto target_layer_i : layers[j]->target_layers_idx) {
            if (target_layer_i == layer_i) {
              layer_i_attenuation += layers[j]->readRecordingStrength(position);
              break;
            }
          }

          if (1.f <= layer_i_attenuation)  {
            layer_i_attenuation = 1.f;
            break;
          }
        }

        last_calculated_attenuation[layer_i] = layer_i_attenuation;
      }
    }

    for (unsigned int i = 0; i < layers.size(); i++) {
      current_attenuation[i] = smoothValue(last_calculated_attenuation[i], current_attenuation[i]);
    }
  }

  inline float read(TimelinePosition position, Layer* recording, RecordParams record_params) {
    updateLayerAttenuations(position);

    float signal_out = 0.f;
    for (unsigned int i = 0; i < layers.size(); i++) {
      if (layers[i]->readableAtPosition(position)) {
        float attenuation = current_attenuation[i];

        if (record_params.active()) {
          for (unsigned int sel_i : recording->target_layers_idx) {
            if (sel_i == i) {
              attenuation += record_params.strength;
              break;
            }
          }
        }

        attenuation = rack::clamp(attenuation, 0.f, 1.f);
        signal_out += layers[i]->readSignal(position) * (1.f - attenuation);
      }
    }

    return signal_out;
  }

  inline std::vector<Layer*> getLayersFromIdx(std::vector<unsigned int> layer_idx) {
    std::vector<Layer*> selected_layers;
    for (auto layer_id : layer_idx) {
      selected_layers.push_back(layers[layer_id]);
    }
    return selected_layers;
  }

  inline float getNumberOfBeatsOfLayerSelection(std::vector<unsigned int> layer_idx) {
    assert(layers.size() != 0);
    assert(layer_idx.size() != 0);
    assert(layer_idx.size() <= layers.size());

    unsigned int max_n_beats = 0;

    for (auto layer : getLayersFromIdx(layer_idx)) {
      if (max_n_beats < layer->n_beats) {
        max_n_beats = layer->n_beats;
      }
    }

    return max_n_beats;
  }
};

} // namespace frame
} // namespace dsp
} // namespace myrisa
