#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;
using namespace kokopellivcv::dsp;

inline bool Engine::phaseDefined() {
  return _use_ext_phase || _phase_oscillator.isSet();
}

inline int findNiceNumberAroundFirstThatFitsIntoSecond(int n1, int n2) {
  assert(n1 < n2);

  while (n2 % n1 != 0) {
    n1++;
    if (n1 == n2) {
      return n1;
    }
  }

  return n1;
}

void Engine::fitLayerIntoCircle(Layer* layer) {
  unsigned int circle_n_beats = _circle.second - _circle.first;
  if (layer->_n_beats == circle_n_beats) {
    return;
  }

  if (layer->_n_beats < circle_n_beats) {
    layer->_n_beats = findNiceNumberAroundFirstThatFitsIntoSecond(layer->_n_beats, circle_n_beats);
  } else {
    unsigned int new_circle_n_beats = circle_n_beats;
    while (new_circle_n_beats < layer->_n_beats) {
      new_circle_n_beats += circle_n_beats;
    }
    layer->_n_beats = new_circle_n_beats;
    _circle.second = _circle.first + new_circle_n_beats;
  }
}

void Engine::endRecording(bool loop, bool create_new_circle) {
  assert(isRecording());
  assert(_recording_layer->_n_beats != 0);

  if (!_phase_oscillator.isSet()) {
    if (_use_ext_phase && _phase_analyzer.getDivisionPeriod() != 0) {
      _phase_oscillator.setFrequency(1 / _phase_analyzer.getDivisionPeriod());
    } else {
      float recording_time = _recording_layer->_in->_samples_per_beat * _sample_time;
      _phase_oscillator.setFrequency(1 / recording_time);
    }
    // printf("-- phase oscillator set with frequency: %f, sample time is: %f\n", _phase_oscillator.getFrequency(), _sample_time);
  }

  if (loop) {
    _recording_layer->_loop = true;
    if (create_new_circle) {
      unsigned int new_circle_n_beats = _recording_layer->_n_beats;
      _circle.second = _circle.first + new_circle_n_beats;
    } else {
      fitLayerIntoCircle(_recording_layer);
    }
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


  // printf("- rec end\n");
  // printf("-- start_beat %d n_beats %d  loop %d samples_per_beat %d layer_i %d\n", _recording_layer->_start_beat, _recording_layer->_n_beats,  _recording_layer->_loop, _recording_layer->_in->_samples_per_beat, layer_i);
  _recording_layer = nullptr;
}

Layer* Engine::newRecording() {
  assert(_record_params.active());
  assert(_recording_layer == nullptr);

  unsigned int start_beat = _timeline_position.beat;
  unsigned int n_beats = 1;

  unsigned int circle_n_beats = _circle.second - _circle.first;
  if (this->_record_params.fix_bounds) {
    start_beat = _circle.first;
    n_beats = circle_n_beats;
  }

  bool shift_circle = !this->_record_params.fix_bounds;
  if (shift_circle) {
    _circle.first = start_beat;
    _circle.second = start_beat + circle_n_beats;
  }

  if (this->_record_params.fix_bounds) {
    if (0 < _timeline.layers.size()) {
      if (_timeline.layers[_active_layer_i]->_loop) {
        n_beats = _timeline.layers[_active_layer_i]->_n_beats;
      } else {
        n_beats = circle_n_beats;
      }
    }
  } else {
    n_beats = _timeline_position.beat - _circle.first + 1;
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

  recording_layer->_circle_before = _circle;

  // printf("Recording Activate:\n");
  // printf("-- start_beat %d n_beats %d loop %d samples per beat %d active layer %d\n", recording_layer->_start_beat, recording_layer->_n_beats, recording_layer->_loop, recording_layer->_in->_samples_per_beat, _active_layer_i);

  return recording_layer;
}

inline void Engine::handleBeatChange(PhaseAnalyzer::PhaseEvent event) {
  assert(phaseDefined());

  bool reached_circle_end = _timeline_position.beat == _circle.second;

  if (reached_circle_end) {
    bool shift_circle = this->isRecording() && !_record_params.fix_bounds;
    if (shift_circle) {
      unsigned int circle_shift = _circle.second - _circle.first;
      _circle.first = _circle.second;
      _circle.second += circle_shift;
    } else {
      // _read_antipop_filter.trigger();
      // TODO doesnt work too well, use antipop trigger instead
      _write_antipop_filter.trigger();
      _timeline_position.beat = _circle.first;
    }
  }

  bool reached_recording_end = this->isRecording() && _recording_layer->_start_beat + _recording_layer->_n_beats <= _timeline_position.beat;
  if (reached_recording_end && !_record_params.fix_bounds) {
    _recording_layer->_n_beats++;
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
    this->endRecording(true, false);
    if (!_record_params.fix_bounds) {
      _record_params.fix_bounds = true;
    }
  }

  if (this->isRecording()) {
    float in = _write_antipop_filter.process(_record_params.readIn());
    _recording_layer->write(_timeline_position, in, _record_params.strength, this->phaseDefined());
  }
}
