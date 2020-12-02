#include "Engine.hpp"

using namespace myrisa::dsp::gko;

inline void Engine::handlePhaseFlip() {
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
        _recording->resizeToLength();
        // TODO set start pos further back if reverse
      }
    }
  }

    // } else if (_mode == RecordParams::Mode::EXTEND) {
      // while (start_beat + n_beats <= beat) {
      //   buffer->resize(buffer->size() + samples_per_beat);
      //   recording_strength->resize(recording_strength->size() + samples_per_beat);
      //   n_beats++;
      // }
  //}
}

inline myrisa::dsp::PhaseAnalyzer::PhaseFlip Engine::advanceTimelinePosition() {
  float new_phase;
  if (_use_ext_phase) {
    new_phase = _ext_phase;
  } else {
    new_phase = _phase_oscillator.step(_sample_time);
  }

  std::pair<int, float> time_beat_and_phase = splitBeatAndPhase(_time);

  PhaseAnalyzer::PhaseFlip phase_flip = _phase_analyzer.process(new_phase, _sample_time);
  if (phase_flip == PhaseAnalyzer::PhaseFlip::BACKWARD && 1 <= time_beat_and_phase.first) {
    _time = time_beat_and_phase.first + new_phase - 1;
  } else if (phase_flip == PhaseAnalyzer::FORWARD) {
    _time = time_beat_and_phase.first + new_phase + 1;
  } else {
    _time = time_beat_and_phase.first + new_phase;
  }

  return phase_flip;
}

void Engine::step() {
  bool phase_defined = (_use_ext_phase || _phase_oscillator.isSet());
  if (phase_defined) {
    PhaseAnalyzer::PhaseFlip phase_flip = advanceTimelinePosition();

    if (phase_flip != PhaseAnalyzer::PhaseFlip::NONE) {
      handlePhaseFlip();
    }
  }

  if (_record.active) {
    record();
  }
}
