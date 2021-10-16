#pragma once

#include "definitions.hpp"
#include "CircleGroup.hpp"

namespace kokopellivcv {
namespace dsp {
namespace circle {

/** Gko - 'God' - the advancer of existence. Moves the music_circle

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

  TimePosition _timeline_position;

  Gko() {
    // FIXME have it set by first member
    _phase_oscillator.setFrequency(1);
  }

  inline void updateTimelinePosition(Parameters* params) {
    // FIXME
    // bool phase_defined = params->_use_ext_phase || _phase_oscillator.isSet();
    // if (phase_defined) {

    float internal_phase = _phase_oscillator.step(params->sample_time);
    _timeline_position.phase = params->use_ext_phase ? params->ext_phase : internal_phase;

    PhaseAnalyzer::PhaseEvent phase_event = _phase_analyzer.process(_timeline_position.phase, params->sample_time);
    if (phase_event != PhaseAnalyzer::PhaseEvent::NONE) {
      if (phase_event == PhaseAnalyzer::PhaseEvent::BACKWARD && 1 <= _timeline_position.beat) {
        _timeline_position.beat--;
      } else if (phase_event == PhaseAnalyzer::PhaseEvent::FORWARD) {
        _timeline_position.beat++;
      }
    }
  }

  inline float advance(CircleGroup* music_circle, Parameters* params) {
    updateTimelinePosition(params);
    return music_circle->step(_timeline_position, params);
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
