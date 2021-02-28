#include "Engine.hpp"

using namespace myrisa::dsp::gko;
using namespace myrisa::dsp;

inline bool Engine::phaseDefined() {
  return _use_ext_phase || _phase_oscillator.isSet();
}

void Engine::endRecording() {
  assert(isRecording());
  assert(_recording_layer->_n_beats != 0);

  if (!_phase_oscillator.isSet()) {
    if (_use_ext_phase && _phase_analyzer.getDivisionPeriod() != 0) {
      _phase_oscillator.setFrequency(1 / _phase_analyzer.getDivisionPeriod());
    } else {
      float recording_time = _recording_layer->_in->_samples_per_beat * _sample_time;
      _phase_oscillator.setFrequency(1 / recording_time);
    }
    printf("-- phase oscillator set with frequency: %f, sample time is: %f\n", _phase_oscillator.getFrequency(), _sample_time);
  }

  _timeline.layers.push_back(_recording_layer);
  _timeline._last_calculated_attenuation.resize(_timeline.layers.size());
  _timeline._current_attenuation.resize(_timeline.layers.size());

  unsigned int layer_i = _timeline.layers.size() - 1;

  if (_select_new_layers) {
    _selected_layers_idx.push_back(layer_i);
  }

  if (_new_layer_active) {
    _active_layer_i = layer_i;
  }

  if (_recording_layer->_loop && _loop_length < _recording_layer->_n_beats) {
    _loop_length = _recording_layer->_n_beats;
  }

  printf("- rec end\n");
  printf("-- start_beat %d n_beats %d  loop %d samples_per_beat %d layer_i %d\n", _recording_layer->_start_beat, _recording_layer->_n_beats,  _recording_layer->_loop, _recording_layer->_in->_samples_per_beat, layer_i);

  _recording_layer = nullptr;
}

Layer* Engine::newRecording() {
  assert(_record_params.active());
  assert(_recording_layer == nullptr);

  unsigned int n_beats = 1;
  unsigned int start_beat = _timeline_position.beat;

  bool new_circle = _record_params.time_frame == TimeFrame::CIRCLE && _record_params.mode == RecordParams::Mode::EXTEND && _read_time_frame == TimeFrame::TIME;
  if (new_circle) {
    _circle.first = start_beat;
    _circle.second = start_beat + 1;
    _loop_length = 1;
  } else if (_record_params.time_frame == TimeFrame::CIRCLE) {
  // TODO
  // if (_record_params.time_frame == TimeFrame::CIRCLE && _read_time_frame == TimeFrame::CIRCLE) {
    start_beat = _circle.first;

    if (_record_params.mode == RecordParams::Mode::DUB) {
      if (0 < _timeline.layers.size() && _timeline.layers[_active_layer_i]->_loop) {
        n_beats = _timeline.layers[_active_layer_i]->_n_beats;
      }
    } else {
      n_beats = _timeline_position.beat - _circle.first + 1;
    }
  }

  int samples_per_beat = 0;
  if (phaseDefined()) {
    if (_use_ext_phase) {
      samples_per_beat = _phase_analyzer.getSamplesPerBeat(_sample_time);
    } else if (_phase_oscillator.isSet()) {
      float beat_period = 1 / _phase_oscillator.getFrequency();
      samples_per_beat = floor(beat_period / _sample_time);
    }
  }

  Layer* recording_layer = new Layer(start_beat, n_beats, _selected_layers_idx, _signal_type, samples_per_beat);

  if (_record_params.time_frame == TimeFrame::CIRCLE) {
    recording_layer->_loop = true;
  }

  printf("Recording Activate:\n");
  printf("-- start_beat %d n_beats %d loop %d samples per beat %d active layer %d\n", recording_layer->_start_beat, recording_layer->_n_beats, recording_layer->_loop, recording_layer->_in->_samples_per_beat, _active_layer_i);

  return recording_layer;
}

inline void Engine::handleBeatChange(PhaseAnalyzer::PhaseEvent event) {
  assert(phaseDefined());

  bool reached_circle_end = _timeline_position.beat == _circle.second;
  if (reached_circle_end) {
    bool grow_circle = this->isRecording() && _record_params.mode == RecordParams::Mode::EXTEND && !(_record_params.time_frame == TimeFrame::TIME && _read_time_frame == TimeFrame::TIME);
    if (grow_circle) {
      _circle.second += _loop_length;
      if (_record_params.time_frame == TimeFrame::CIRCLE) {
        _recording_layer->_n_beats += _loop_length;
      } else {
        _recording_layer->_n_beats += 1;
      }
    } else {
      bool skip_back_to_circle_start = _read_time_frame == TimeFrame::CIRCLE;
      if (skip_back_to_circle_start) {
        _read_antipop_filter.trigger();
        _write_antipop_filter.trigger();
        _timeline_position.beat = _circle.first;
        if (_options.create_new_layer_on_skip_back && this->isRecording()) {
          this->endRecording();
          _recording_layer = this->newRecording();
        }
      } else {
        unsigned int circle_shift = _circle.second - _circle.first;
        _circle.first = _circle.second;
        _circle.second += circle_shift;

      }
    }
  }

  bool reached_recording_end = this->isRecording() && _recording_layer->_start_beat + _recording_layer->_n_beats <= _timeline_position.beat;
  if (reached_recording_end) {
    bool create_new_dub = _record_params.mode == RecordParams::Mode::DUB && _record_params.time_frame == TimeFrame::CIRCLE && _read_time_frame == TimeFrame::TIME;
    bool overwrite = _record_params.mode == RecordParams::Mode::DUB && _record_params.time_frame == TimeFrame::CIRCLE && _read_time_frame == TimeFrame::CIRCLE;
    if (create_new_dub) {
      this->endRecording();
      _recording_layer = this->newRecording();
    } else if (!overwrite) {
      _recording_layer->_n_beats++;
    }
  }
}


inline PhaseAnalyzer::PhaseEvent Engine::advanceTimelinePosition() {
  float internal_phase = _phase_oscillator.step(_sample_time);
  _timeline_position.phase = _use_ext_phase ? _ext_phase : internal_phase;

  PhaseAnalyzer::PhaseEvent phase_event = _phase_analyzer.process(_timeline_position.phase, _sample_time);

  unsigned int new_beat = _timeline_position.beat;
  if (phase_event == PhaseAnalyzer::PhaseEvent::BACKWARD && 1 <= _timeline_position.beat) {
    new_beat = _timeline_position.beat - 1;
  } else if (phase_event == PhaseAnalyzer::PhaseEvent::FORWARD) {
    new_beat = _timeline_position.beat + 1;
  }

  if (phase_event == PhaseAnalyzer::PhaseEvent::DISCONTINUITY && _options.use_antipop) {
    _read_antipop_filter.trigger();
  }

  _timeline_position.beat = new_beat;

  return phase_event;
}

void Engine::step() {
  if (this->phaseDefined()) {
    PhaseAnalyzer::PhaseEvent phase_event = this->advanceTimelinePosition();
    if (phase_event == PhaseAnalyzer::PhaseEvent::FORWARD || phase_event == PhaseAnalyzer::PhaseEvent::BACKWARD) {
      this->handleBeatChange(phase_event);
    }
  }

  if (!this->isRecording() && _record_params.active()) {
    _recording_layer = this->newRecording();
    _write_antipop_filter.trigger();
  } else if (this->isRecording() && !_record_params.active()) {
    this->endRecording();
  }

  if (this->isRecording()) {
    float in = _write_antipop_filter.process(_record_params.readIn());
    _recording_layer->write(_timeline_position, in, _record_params.strength, this->phaseDefined());
  }
}
