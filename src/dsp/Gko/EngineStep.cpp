#include "Engine.hpp"

using namespace myrisa::dsp::gko;
using namespace myrisa::dsp;

inline void Engine::write() {
  assert(_record_params.active());
  assert(_recording != nullptr);

  bool phase_defined = (_use_ext_phase || _phase_oscillator.isSet());
  if (!phase_defined) {
    _recording->pushBack(_record_params.in, _record_params.strength);
    _recording->samples_per_beat++;
  } else if (_recording->writableAtPosition(_timeline_position)) {
    _recording->write(_timeline_position, _record_params.in, _record_params.strength);
  } else {
    printf("Not writable \n");
  }
}

inline void Engine::endRecording() {
    assert(_recording != nullptr);
    assert(_recording->n_beats != 0.f);
    assert(_recording->signal->size() != 0);
    assert(_recording_active);

    float recording_time = _recording->signal->size() * _sample_time;

    printf("Recording De-Activate\n");
    printf("-- start_beat %d n_beats %d size %d recording time %fs loop %d samples_per_beat %d\n", _recording->start_beat, _recording->n_beats, _recording->signal->size(), recording_time, _recording->loop, _recording->samples_per_beat);

    if (!_phase_oscillator.isSet()) {
      if (_use_ext_phase && _phase_analyzer.getDivisionPeriod() != 0) {
        _phase_oscillator.setFrequency(1 / _phase_analyzer.getDivisionPeriod());
      } else {
        _phase_oscillator.setFrequency(1 / recording_time);
      }
      printf("-- phase oscillator set with frequency: %f\n", _phase_oscillator.getFrequency());
    }

    _timeline.layers.push_back(_recording);

    _recording_active = false;
    _recording = nullptr;
}

inline void Engine::beginRecording() {
  assert(_record_params.active());
  assert(_recording == nullptr);

  unsigned int n_beats = 1;
  if (_record_params.mode == RecordParams::Mode::DUB && 0 < _record_params.selected_layers.size()) {
    n_beats = _timeline.getNumberOfBeatsOfLayerSelection(_record_params.selected_layers);
  }

  unsigned int start_beat = _timeline_position.beat;
  if (_record_params.mode == RecordParams::Mode::EXTEND) {
    start_beat = _timeline_position.beat + std::round(_timeline_position.phase);
  }

  _recording = new Layer(start_beat, n_beats, _record_params.selected_layers);

  bool phase_defined = (_use_ext_phase || _phase_oscillator.isSet());
  if (phase_defined) {
    if (_use_ext_phase) {
      _recording->samples_per_beat = _phase_analyzer.getSamplesPerDivision();
    } else if (_phase_oscillator.isSet()) {
      float beat_period = 1 / _phase_oscillator.getFrequency();
      _recording->samples_per_beat = floor(beat_period / _sample_time);
    }
    _recording->resizeToLength();
  }

  if (_record_params.time_frame != TimeFrame::TIMELINE) {
    _recording->loop = true;
  }

  _recording_active = true;

  printf("Recording Activate:\n");
  printf("-- start_beat %d n_beats %d loop %d samples per beat %d\n", _recording->start_beat, _recording->n_beats, _recording->loop, _recording->samples_per_beat);
}

inline void Engine::handlePhaseFlip(PhaseAnalyzer::PhaseFlip flip) {
  bool phase_defined = (_use_ext_phase || _phase_oscillator.isSet());
  assert(phase_defined);

  if (_recording) {
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

  if (_recording_active != _record_params.active()) {
    if (!_recording_active) {
      beginRecording();
    } else {
      endRecording();
    }
  }

  if (_recording_active) {
    write();
  }
}
