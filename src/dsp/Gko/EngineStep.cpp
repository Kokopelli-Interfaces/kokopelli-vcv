#include "Engine.hpp"

using namespace myrisa::dsp::gko;

inline void Engine::handlePhaseFlip() {
  if (_manifest.active && _manifest.mode == Manifest::Mode::DUB) {
    bool reached_manifestation_end = _manifestation->start + _manifestation->length <= _time;
    if (reached_manifestation_end) {
      printf("END Manifest via DUB\n");
      // printf("-- Layer start: %f length %ld loop %b rec_size: %l\n", _manifestation->start, _manifestation->length, _manifestation->loop, _manifestation->signal->size());
      _timeline.layers.push_back(_manifestation);
    }
  }

    // } else if (_mode == Manifest::Mode::EXTEND) {
      // while (start_beat + n_beats <= beat) {
      //   buffer->resize(buffer->size() + samples_per_beat);
      //   manifestation_strength->resize(manifestation_strength->size() + samples_per_beat);
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

  if (_manifest.active) {
    manifest();
  }
}
