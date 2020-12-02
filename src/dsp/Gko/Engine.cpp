#include "Engine.hpp"

using namespace myrisa::dsp::gko;

void Engine::endRecording() {
    assert(_recording != nullptr);
    assert(_recording->length != 0.f);
    assert(_recording->signal->size() != 0);

    float recording_time = _recording->signal->size() * _sample_time;

    printf("Record De-Activate\n");
    printf("-- Recording start %f length %f size %d recording time %fs loop %d samples_per_beat %d\n", _recording->start, _recording->length, _recording->signal->size(), recording_time, _recording->loop, _recording->samples_per_beat);

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
  assert(_record.active);
  assert(_recording == nullptr);

  float length = 1.f;
  if (_record.mode == RecordParams::Mode::DUB && 0 < _record.selected_layers.size()) {
    length = _timeline.getLengthOfLayers(_record.selected_layers);
  }

  float start_time;
  if (_record.mode == RecordParams::Mode::EXTEND) {
    start_time = std::round(_time);
  } else {
    start_time = std::floor(_time);
  }

  _recording = new Layer(start_time, length, _record.selected_layers);

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

  if (_record.time_frame != TimeFrame::TIMELINE) {
    _recording->loop = true;
  }

  printf("-- Recording start %f initial length %f loop %d samples per beat %d\n", _recording->start, _recording->length, _recording->loop, _recording->samples_per_beat);
}

void Engine::setRecordStrength(float strength) {
  _record.strength = strength;

  bool record_activate = !_record.active && _recordActiveThreshold <= strength;
  bool record_deactivate = _record.active && strength < _recordActiveThreshold ;

  if (record_activate) {
    printf("Record Activate\n");
    _record.active = true;
    _record.selected_layers = _selected_layers;
    beginRecording();
  } else if (record_deactivate) {
    _record.active = false;
    endRecording();
  }
}

void Engine::setRecordTimeFrame(TimeFrame time_frame) {
  if (_record.active) {
    assert(_recording != nullptr);

    if (time_frame != TimeFrame::TIMELINE) {
      _recording->loop = true;
    }
  }

  _record.time_frame = time_frame;
}

void Engine::setRecordMode(RecordParams::Mode mode) {
  if (mode == _record.mode) {
    return;
  }

  // TODO transitions
  _record.mode = mode;
}
