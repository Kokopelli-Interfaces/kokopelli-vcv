#pragma once

#include "definitions.hpp"
#include "util/math.hpp"
#include "Layer.hpp"
#include "Channel.hpp"
#include <string>

namespace myrisa {
namespace dsp {
namespace gko {

class LayerManager {
  std::vector<Layer*> layers;

  /** read only */

  // seperate attenuation calculation for each channel?
  // I suppose. some attenuations only target some channels
  // and more can be added.
  std::vector<float> _current_layer_attenuations;
  std::vector<float> _last_calculated_layer_attenuations;
  rack::dsp::ClockDivider _attenuation_calculator_divider;

  LayerManager() {
    _attenuation_calculator_divider.setDivision(2000);
  }

  static inline float smoothValue(float current, float old) {
    const float lambda = 30.f / 44100;
    return old + (current - old) * lambda;
  }

  inline unsigned int getNumberOfCircleBeats(TimePosition position) {
    vector<unsigned int> layer_n_beats;
    for (auto layer : layers) {
      if (layer->readableAtPosition(position) && layer->_loop) {
        layer_n_beats.push_back(layer->_n_beats);
      }
    }

    return myrisa::util::lcm(layer_n_beats);
  }

  inline unsigned int getCircleStartBeat(TimePosition position) {
    unsigned int start_beat = 0;
    for (auto layer : layers) {
      if (layer->readableAtPosition(position) && layer->_loop && start_beat < layer->_start_beat) {
        start_beat = layer->_start_beat;
      }
    }
    return start_beat;
  }

  inline void updateLayerAttenuations(TimePosition position) {
    if (_attenuation_calculator_divider.process()) {
      for (unsigned int layer_i = 0; layer_i < layers.size(); layer_i++) {
        float layer_i_attenuation = 0.f;
        for (unsigned int j = layer_i + 1; j < layers.size(); j++) {
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

        _last_calculated_layer_attenuations[layer_i] = layer_i_attenuation;
      }
    }

    for (unsigned int i = 0; i < layers.size(); i++) {
      _current_layer_attenuations[i] = smoothValue(_last_calculated_layer_attenuations[i], _current_layer_attenuations[i]);
    }
  }

  inline float read(TimePosition position, Layer* recording, RecordInterface record_interface) {
    updateLayerAttenuations(position);

    // FIXME multiple recordings in layer, have loop and array of types
  myrisa::dsp::SignalType signal_type = myrisa::dsp::SignalType::AUDIO;
    if (0 < layers.size()) {
      signal_type = layers[0]->_in->_signal_type;
    }

    float signal_out = 0.f;
    for (unsigned int i = 0; i < layers.size(); i++) {
      if (layers[i]->readableAtPosition(position)) {
        float attenuation = _current_layer_attenuations[i];

        if (record_interface.active()) {
          for (unsigned int sel_i : recording->target_layers_idx) {
            if (sel_i == i) {
              attenuation += record_interface.strength;
              break;
            }
          }
        }

        attenuation = rack::clamp(attenuation, 0.f, 1.f);
        float layer_out = layers[i]->readSignal(position);
        layer_out = myrisa::dsp::attenuate(layer_out, attenuation, signal_type);
        signal_out = myrisa::dsp::sum(signal_out, layer_out, signal_type);
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

} // namespace gko
} // namespace dsp
} // namepsace myrisa
