#pragma once

#include "dsp/misc/PhaseAnalyzer.hpp"
#include "dsp/misc/PhaseOscillator.hpp"
#include "definitions.hpp"
#include "Timeline.hpp"
#include "NodeGroup.hpp"

namespace kokopellivcv {
namespace emernet {

/** Gko 'the advancer of the emernet'.

    stores the movement of light on a timeline

    v~~LOVE-G1~~___~~
    ---------------------------------
              ^--LOVE~G2~~~_~

    IDEA: potentialities[timelines]
  */
struct Gko {
  Timeline _timeline;
  PhaseOscillator _phase_oscillator;
  PhaseAnalyzer _phase_analyzer;

  Time _play_head;

  bool _loving = false;

  Gko() {
    // FIXME have it set by first member
    _phase_oscillator.setFrequency(1);
  }

  inline void updateTimelinePosition(Inputs inputs, Parameters params) {
    // FIXME
    // bool phase_defined = params.use_ext_phase || _phase_oscillator.isSet();
    // if (phase_defined) {
    float internal_phase = _phase_oscillator.step(params.sample_time);
    _play_head.phase = params.use_ext_phase ? inputs.ext_phase : internal_phase;

    PhaseAnalyzer::PhaseEvent phase_event = _phase_analyzer.process(_play_head.phase, params.sample_time);
    if (phase_event != PhaseAnalyzer::PhaseEvent::NONE) {
      if (phase_event == PhaseAnalyzer::PhaseEvent::BACKWARD && 1 <= _play_head.beat) {
        _play_head.beat--;
      } else if (phase_event == PhaseAnalyzer::PhaseEvent::FORWARD) {
        _play_head.beat++;
      }
    }
  }

  inline void startLovingLiveNode(EmernetNode* live_node) {
    live_node->_is_being_loved = true;
    live_node->_start = _play_head;
  }

  inline void endLovingLiveNode(EmernetNode* live_node, Emernet *emernet) {
    EmernetNode* capture_node = new EmernetNode();
    capture_node->_type = NodeType::CAPTURE;
    capture_node->_group = live_node->_group;
    capture_node->_start = live_node->_start;
    capture_node->_end = _play_head;

    emernet->_all_nodes.push_back(capture_node);
    // TODO nodes with light
    // TODO append light movement

    live_node->_is_being_loved = false;
  }

  inline void advanceLiveNode(EmernetNode* live_node, Emernet *emernet, Inputs inputs, Parameters params) {
    if (!live_node->_is_being_loved && inputs->loveActive()) {
      startLovingLiveNode(live_node);
    } else if (live_node->_is_being_loved && !inputs->loveActive()) {
      endLovingLiveNode(live_node, emernet);
    }
  }

  inline void advanceCaptureNode(EmernetNode* capture_node, Emernet *emernet, Inputs inputs, Parameters params) {

  }

  inline void advance(Emernet* emernet, Inputs inputs, Parameters params) {
    // walk emernet and advance everyone
    // updates light and play_heads for all nodes
    for (auto node : emernet->_all_nodes) {
      assert(node->_group != nullptr);

      if (node->_type == NodeType::LIVE) {
        advanceLiveNode(node, emernet, inputs, params);
      } else (node->_type == NodeType::CAPTURE) {
        advanceCaptureNode(node, inputs, params);
      }
    }

    updateTimelinePosition(inputs, params);
  }
};

} // namespace emernet
} // namespace kokopellivcv
