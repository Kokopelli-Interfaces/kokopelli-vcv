#include "Engine.hpp"

using namespace myrisa::dsp::gko;
using namespace myrisa::dsp;

inline void Engine::handlePhaseFlip(PhaseAnalyzer::PhaseFlip flip) {
  if (flip == PhaseAnalyzer::PhaseFlip::NONE) {
    return;
  }

  bool phase_defined = (_use_ext_phase || _phase_oscillator.isSet());
  assert(phase_defined);

  if (_record.active) {
    assert(_recording != nullptr);
    assert(_recording->samples_per_beat != 0);

    bool reached_recording_end = _recording->start + _recording->length <= _time;
    if (reached_recording_end) {
      if (_record.mode == RecordParams::Mode::DUB) {
      printf("DUB END\n");
        endRecording();
        beginRecording();
      } else if (_record.mode == RecordParams::Mode::EXTEND) {
        _recording->length = _recording->length + 1.f;
        printf("extend recording to: %f\n", _recording->length);
        _recording->resizeToLength();
        // TODO set start pos further back if reverse
      }
    }
  }
}

inline PhaseAnalyzer::PhaseFlip Engine::advanceTimelinePosition() {
  float new_phase;
  if (_use_ext_phase) {
    new_phase = _ext_phase;
  } else {
    new_phase = _phase_oscillator.step(_sample_time);
  }

  float beat = _time - rack::math::eucMod(_time, 1.0f);

  PhaseAnalyzer::PhaseFlip phase_flip = _phase_analyzer.process(new_phase, _sample_time);
  if (phase_flip == PhaseAnalyzer::PhaseFlip::BACKWARD && 1 <= beat) {
    _time = beat + new_phase - 1.f;
  } else if (phase_flip == PhaseAnalyzer::PhaseFlip::FORWARD) {
    _time = beat + new_phase + 1;
  } else {
    _time = beat + new_phase;
  }

  return phase_flip;
}

void Engine::step() {
  bool phase_defined = (_use_ext_phase || _phase_oscillator.isSet());
  if (phase_defined) {
    handlePhaseFlip(advanceTimelinePosition());
  }

  if (_record.active) {
    record();
  }
}
