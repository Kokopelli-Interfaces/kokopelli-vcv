#include "Engine.hpp"

using namespace myrisa::dsp::gko;
using namespace myrisa::dsp;

inline void Engine::handlePhaseFlip(PhaseAnalyzer::PhaseFlip flip) {
  bool phase_defined = (_use_ext_phase || _phase_oscillator.isSet());
  assert(phase_defined);

  if (_record_params.active) {
    assert(_recording != nullptr);
    assert(_recording->samples_per_beat != 0);

    bool reached_recording_end = _recording->start_beat + _recording->n_beats <= _timeline_position.beat;
    if (reached_recording_end) {
      if (_record_params.mode == RecordParams::Mode::DUB) {
      printf("DUB END\n");
        endRecording();
        beginRecording();
      } else if (_record_params.mode == RecordParams::Mode::EXTEND) {
        _recording->n_beats = _recording->n_beats + 1.f;
        printf("extend recording to: %d\n", _recording->n_beats);
        _recording->resizeToLength();
        // TODO set start pos further back if reverse
      }
    }
  }
}

inline PhaseAnalyzer::PhaseFlip Engine::advanceTimelinePosition() {
  float internal_phase = _phase_oscillator.step(_sample_time);
  _timeline_position.phase = _use_ext_phase ? _ext_phase : internal_phase;

  PhaseAnalyzer::PhaseFlip phase_flip = _phase_analyzer.process(_timeline_position.phase, _sample_time);
  if (phase_flip == PhaseAnalyzer::PhaseFlip::BACKWARD && 1 <= _timeline_position.beat) {
    _timeline_position.beat--;
  } else if (phase_flip == PhaseAnalyzer::PhaseFlip::FORWARD) {
    _timeline_position.beat++;
  }

  return phase_flip;
}

void Engine::step() {
  bool phase_defined = (_use_ext_phase || _phase_oscillator.isSet());
  if (phase_defined) {
    PhaseAnalyzer::PhaseFlip phase_flip = advanceTimelinePosition();

    if (phase_flip != PhaseAnalyzer::PhaseFlip::NONE) {
      handlePhaseFlip(phase_flip);
    }
  }

  if (_record_params.active) {
    record();
  }
}
