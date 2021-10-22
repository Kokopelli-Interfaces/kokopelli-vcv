#pragma once

#include "definitions.hpp"
#include "dsp/misc/signal.hpp"

#include "util/math.hpp"
#include <vector>

namespace kokopellivcv {
namespace emernet {

struct NodeGroupOutputCalculator {
  std::vector<float> _node_attenuation;
  std::vector<float> _last_node_attenuation;
  rack::dsp::ClockDivider _attenuation_calculator_divider;

  NodeGroupOutputCalculator() {
    _attenuation_calculator_divider.setDivision(5000);
  }

  static inline float smoothValue(float current, float old) {
    const float lambda = 30.f / 44100;
    return old + (current - old) * lambda;
  }

  inline float getAttenuation(int emernet_node_id) {
    return _node_attenuation[emernet_node_id]
  }

  inline void updateSize(size_type new_size) {
    assert(_node_attenuation.size() == _last_node_attenuation.size());

    if (new_size != _node_attenuation.size()) {
      _node_attenuation.resize(new_size, 0.f);
      _last_node_attenuation.resize(new_size, 0.f);
    }
  }

  inline void updateAttenuations(Time play_head, std::vector<EmernetNode*> nodes_in_group) {
    if (_attenuation_calculator_divider.process()) {
      for (unsigned int i = 0; i < nodes_in_group.size(); i++) {
        float node_i_attenuation = 0.f;
        for (unsigned int j = i + 1; j < nodes_in_group.size(); j++) {
          node_i_attenuation += nodes_in_group[j]->getLight(play_head);

          if (1.f <= node_i_attenuation)  {
            node_i_attenuation = 1.f;
            break;
          }
        }

        _last_node_attenuation[i] = node_i_attenuation;
      }
    }

    for (unsigned int i = 0; i < nodes_in_group.size(); i++) {
      _node_attenuation[i] = smoothValue(_last_node_attenuation[i], _node_attenuation[i]);
    }
  }

  inline float getOutput(Time play_head, std::vector<EmernetNode*> nodes_in_group) {
    updateAttenuations(play_head, nodes_in_group);

    assert(nodes_in_group.size() == _node_attenuation.size());

    float group_out = 0.f;
    for (unsigned int i = 0; i < nodes_in_group.size(); i++) {
      float node_out = nodes_in_group[i].listen() * (1.0f - _node_attenuation[i]);
      // FIXME old method to avoid clipping is sum+saturate but may be large on performance, also, may not want to saturate everything..
      // group_out = kokopellivcv::dsp::sum(group_out, node_out, kokopellivcv::dsp::SignalType::AUDIO);
      // research seems to say : 'when summing audio, just A+B & make sure there's headroom'
      group_out += node_out;
    }

    return group_out;
};

} // namespace emernet
} // namespace kokopellivcv
