#include "Engine.hpp"

using namespace myrisa::dsp::gko;

void Engine::endRecording() {
    assert(_recording != nullptr);
    assert(_recording->n_beats != 0.f);
    assert(_recording->signal->size() != 0);

    float recording_time = _recording->signal->size() * _sample_time;

    printf("Recording De-Activate\n");
    printf("-- start_beat %d length %d size %d recording time %fs loop %d samples_per_beat %d\n", _recording->start_beat, _recording->n_beats, _recording->signal->size(), recording_time, _recording->loop, _recording->samples_per_beat);

    if (!_phase_oscillator.isSet()) {
      if (_use_ext_phase && _phase_analyzer.getDivisionPeriod() != 0) {
        _phase_oscillator.setFrequency(1 / _phase_analyzer.getDivisionPeriod());
      } else {
        _phase_oscillator.setFrequency(1 / recording_time);
      }
      printf("-- phase oscillator set with frequency: %f\n", _phase_oscillator.getFrequency());
    }

    _timeline.layers.push_back(_recording);

    _recording = nullptr;
}

void Engine::beginRecording() {
  assert(_record_params.active);
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

  printf("Recording Activate:\n");
  printf("-- start_beat %d length %d loop %d samples per beat %d\n", _recording->start_beat, _recording->n_beats, _recording->loop, _recording->samples_per_beat);
}

void Engine::setRecordStrength(float strength) {
  _record_params.strength = strength;

  bool record_activate = !_record_params.active && _recordActiveThreshold <= strength;
  bool record_deactivate = _record_params.active && strength < _recordActiveThreshold ;

  if (record_activate) {
    _record_params.active = true;
    _record_params.selected_layers = _selected_layers;
    beginRecording();
  } else if (record_deactivate) {
    _record_params.active = false;
    endRecording();
  }
}

void Engine::setRecordTimeFrame(TimeFrame time_frame) {
  if (_record_params.active) {
    assert(_recording != nullptr);
    if (time_frame != TimeFrame::TIMELINE) {
      _recording->loop = true;
    }
  }

  _record_params.time_frame = time_frame;
}

void Engine::setRecordMode(RecordParams::Mode mode) {
  if (mode == _record_params.mode) {
    return;
  }

  // TODO transitions
  _record_params.mode = mode;
}
