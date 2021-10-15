#pragma once

#include "Layer.hpp"
#include "definitions.hpp"
#include "util/math.hpp"
#include <vector>

namespace kokopellivcv {
namespace dsp {
namespace circle {

/**
   The Timeline is the top level structure for content.
*/
struct Timeline {
  std::vector<Layer*> layers;

  float active_layer_out = 0.f;

  /** read only */

  std::vector<float> _current_attenuation;
  std::vector<float> _last_calculated_attenuation;
  rack::dsp::ClockDivider _attenuation_calculator_divider;

  Timeline() {
    _attenuation_calculator_divider.setDivision(2000);
  }

  static inline float smoothValue(float current, float old) {
    const float lambda = 30.f / 44100;
    return old + (current - old) * lambda;
  }

  inline bool atEnd(TimePosition position) {
    if (layers.size() == 0) {
      return true;
    }

    unsigned int last_layer_i = layers.size()-1;
    return layers[last_layer_i]->_start_beat + layers[last_layer_i]->_n_beats <= position.beat + 1;
  }

  inline unsigned int getLayerIndexForPosition(TimePosition position) {
    if (layers.size() == 0) {
      return 0;
    }

    unsigned int layer_i = layers.size()-1;
    for (int i = layers.size()-1; 0 <= i; i--) {
      if (layers[i]->_start_beat <= position.beat) {
        break;
      }

      layer_i = i;
    }

    return layer_i;
  }

  inline void updateLayerAttenuations(TimePosition position) {
    if (_attenuation_calculator_divider.process()) {
      for (unsigned int layer_i = 0; layer_i < layers.size(); layer_i++) {
        float layer_i_attenuation = 0.f;
        for (unsigned int j = layer_i + 1; j < layers.size(); j++) {
          for (auto target_layer_i : layers[j]->target_layers_idx) {
            if (target_layer_i == layer_i) {
              layer_i_attenuation += layers[j]->readRecordingLove(position);
              break;
            }
          }

          if (1.f <= layer_i_attenuation)  {
            layer_i_attenuation = 1.f;
            break;
          }
        }

        _last_calculated_attenuation[layer_i] = layer_i_attenuation;
      }
    }

    for (unsigned int i = 0; i < layers.size(); i++) {
      _current_attenuation[i] = smoothValue(_last_calculated_attenuation[i], _current_attenuation[i]);
    }
  }

  inline float readRawLayers(TimePosition position, std::vector<unsigned int> layers_idx) {
    float signal_out = 0.f;

    // FIXME multiple recordings in layer, have loop and array of types
    kokopellivcv::dsp::SignalType signal_type = kokopellivcv::dsp::SignalType::AUDIO;
    if (0 < layers.size()) {
      signal_type = layers[0]->_in->_signal_type;
    }

    for (auto layer_i : layers_idx) {
      if (layers.size() < layer_i) {
        continue;
      }

      float layer_out = layers[layer_i]->readSignal(position);
      signal_out = kokopellivcv::dsp::sum(signal_out, layer_out, signal_type);
    }

    return signal_out;
  }

  inline float read(TimePosition position, Layer* recording, RecordParams record_params, unsigned int active_layer_i) {
    updateLayerAttenuations(position);

    // FIXME multiple recordings in layer, have loop and array of types
    kokopellivcv::dsp::SignalType signal_type = kokopellivcv::dsp::SignalType::AUDIO;
    if (0 < layers.size()) {
      signal_type = layers[0]->_in->_signal_type;
    }

    float signal_out = 0.f;
    for (unsigned int i = 0; i < layers.size(); i++) {
      if (layers[i]->readableAtPosition(position)) {
        float attenuation = _current_attenuation[i];

        if (record_params.active()) {
          for (unsigned int sel_i : recording->target_layers_idx) {
            if (sel_i == i) {
              attenuation += record_params.new_love;
              break;
            }
          }
        }

        attenuation = rack::clamp(attenuation, 0.f, 1.f);
        float layer_out = layers[i]->readSignal(position);
        layer_out = kokopellivcv::dsp::attenuate(layer_out, attenuation, signal_type);
        if (i == active_layer_i) {
          active_layer_out = layer_out;
        }

        signal_out = kokopellivcv::dsp::sum(signal_out, layer_out, signal_type);
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
      if (max_n_beats < layer->_n_beats) {
        max_n_beats = layer->_n_beats;
      }
    }

    return max_n_beats;
  }

  inline unsigned int getNumberOfCircleBeats(TimePosition position) {
    unsigned int max_n_beats = 0;
    for (auto layer: layers) {
      if (layer->readableAtPosition(position) && layer->_loop && max_n_beats < layer->_n_beats) {
        max_n_beats = layer->_n_beats;
      }
    }

    return max_n_beats;
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
