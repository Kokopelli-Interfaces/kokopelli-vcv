#pragma once

#include "NodeGroupOutputCalculator.hpp"
#include "definitions.hpp"
#include "dsp/misc/signal.hpp"
#include "util/math.hpp"
#include <vector>

namespace kokopellivcv {
namespace emernet {

struct EmernetNodeGroup {
  std::vector<EmernetNode*> _nodes_in_group;
  NodeGroupOutputCalculator _output_calculator;

  Time _period;
  Time _play_head;

  // TODO recorded by gko
  float _light = 0.f;

  inline unsigned int getNumberOfNodes() {
    return _nodes_in_group.size();
  }

  inline void addNode(EmernetNode *node) {
    _nodes_in_group.push_back(node);
    _output_calculator.updateSize(_nodes_in_group.size());
  }

  // TODO use PhaseDivider
  // inline Time getPlayHead(Time play_head) {
  //   _play_head.phase = play_head.phase;
  //   if (_last_play_head.beat != play_head.beat) {
  //     unsigned int n_group_beats = this->getPeriod();
  //     if (n_group_beats != 0) {
  //       _play_head.beat = _last_play_head.beat % n_group_beats;
  //     } else {
  //       _play_head.beat = 0;
  //     }
  //   }
  //   _last_play_head = play_head;
  // }

  inline float listen() {
    return _output_calculator.getOutput(_play_head, _nodes_in_group);
    // _output_calculator.step(play_head, _nodes_in_group);
  }

  inline float getLight(Time play_head) {
    // TODO read END love of _node 0 if past buffer so for easier transitions
    return _light;
  }

  inline unsigned int getPeriod() {
    // assertInvariant();

    unsigned int max_n_beats = 0;
    for (auto node: _nodes_in_group) {
      if (node->isActive() && max_n_beats < node->getPeriod()) {
        max_n_beats = node->getPeriod();
      }
    }

    return max_n_beats;
  }

  inline bool isActive() {
    for (auto node: _nodes_in_group) {
      if (node->isActive()) {
        return true;
      }
    }
    return false;
  }

  inline void assertInvariant() {
    // clock ?
    // assert group p = max(group nodes p)
    // assert group p % (any group node) = 0
    assert(0 < _nodes_in_group.size());
  }
};

} // namespace emernet
} // namespace kokopellivcv
