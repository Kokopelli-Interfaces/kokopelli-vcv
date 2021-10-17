#pragma once

#include "dsp/PhaseAnalyzer.hpp"
#include "dsp/PhaseOscillator.hpp"
#include "definitions.hpp"
#include "CircleGroup.hpp"

namespace kokopellivcv {
namespace dsp {
namespace circle {

/** Gko - 'God' - the advancer of existence. Creates the next frame of the music circle

    Records Love movement on timeline

    v~~LOVE-G1~~___~~
    ---------------------------------
          ^--LOVE~G2~~~_~

    IDEA: potentialities[timelines]

  This means timeline is handled by external unit
  Members are relative to groups
  Do not need timelinepositions
  AFTERALL, what if you want to go to 2->3->2? it skips back in timeline now! but this is not what we need.
  */
struct Gko {
  PhaseOscillator _phase_oscillator;
  PhaseAnalyzer _phase_analyzer;

  TimePosition _play_head;

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

  inline float advance(CircleGroup* music_circle, Inputs inputs, Parameters params) {
    updateTimelinePosition(inputs, params);
    return music_circle->step(_play_head, inputs, params);
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
