#include "Engine.hpp"

using namespace tribalinterfaces::dsp::circle;
using namespace tribalinterfaces::dsp;

inline bool Engine::phaseDefined() {
  return _use_ext_phase || _phase_oscillator.isSet();
}

// -1 is arbitrary card, 0 is green, 1 is red
bool Engine::checkState(int reflect, int previous_member, int next_member) {
  if ((reflect == 1 && _reflect != false) || (reflect == 0 && _reflect != true)) {
    return false;
  }

  if ((previous_member == 1 && _record_params.previous_member) || (previous_member == 0 && !_record_params.previous_member)) {
    return false;
  }

  if ((next_member == 1 && _record_params.next_member) || (next_member == 0 && !_record_params.next_member)) {
    return false;
  }

  return true;
}

void Engine::endRecording() {
  assert(isRecording());
  assert(_recording_member->_n_beats != 0);

  if (!_phase_oscillator.isSet()) {
    if (_use_ext_phase && _phase_analyzer.getDivisionPeriod() != 0) {
      _phase_oscillator.setFrequency(1 / _phase_analyzer.getDivisionPeriod());
    } else {
      float recording_time = _recording_member->_in->_samples_per_beat * _sample_time;
      _phase_oscillator.setFrequency(1 / recording_time);
    }
    printf("-- phase oscillator set with frequency: %f, sample time is: %f\n", _phase_oscillator.getFrequency(), _sample_time);
  }

  _circle.members.push_back(_recording_member);
  _circle._last_calculated_attenuation.resize(_circle.members.size());
  _circle._current_attenuation.resize(_circle.members.size());

  unsigned int member_i = _circle.members.size() - 1;

  if (_select_new_members) {
    _selected_members_idx.push_back(member_i);
  }

  if (_new_member_active) {
    _active_member_i = member_i;
  }

  if (_recording_member->_loop && _loop_length < _recording_member->_n_beats) {
    _loop_length = _recording_member->_n_beats;
  }

  printf("- rec end\n");
  printf("-- start_beat %d n_beats %d  loop %d samples_per_beat %d member_i %d\n", _recording_member->_start_beat, _recording_member->_n_beats,  _recording_member->_loop, _recording_member->_in->_samples_per_beat, member_i);

  _recording_member = nullptr;
}

Member* Engine::newRecording() {
  assert(_record_params.active());
  assert(_recording_member == nullptr);

  unsigned int start_beat = _circle_position.beat;
  unsigned int n_beats = 1;
  if (this->_record_params.previous_member && this->_record_params.next_member) {
    start_beat = _loop.first;
    n_beats = _loop_length;
  }

  bool shift_circle = !this->_record_params.previous_member;
  if (shift_circle) {
    _loop.first = start_beat;
    _loop.second = start_beat + _loop_length;
  }

  if (this->_record_params.next_member) {
    if (this->_record_params.previous_member) {
      if (0 < _circle.members.size()) {
        if (_circle.members[_active_member_i]->_loop) {
          n_beats = _circle.members[_active_member_i]->_n_beats;
        } else {
          n_beats = _loop_length;
        }
      }
    } else {
      n_beats = _circle_position.beat - _loop.first + 1;
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

  Member* recording_member = new Member(start_beat, n_beats, _selected_members_idx, _signal_type, samples_per_beat);

  if (this->_record_params.next_member) {
    recording_member->_loop = true;
  }

  // printf("Recording Activate:\n");
  // printf("-- start_beat %d n_beats %d loop %d samples per beat %d active member %d\n", recording_member->_start_beat, recording_member->_n_beats, recording_member->_loop, recording_member->_in->_samples_per_beat, _active_member_i);

  return recording_member;
}

inline void Engine::handleBeatChange(PhaseAnalyzer::PhaseEvent event) {
  assert(phaseDefined());

  bool reached_circle_end = _circle_position.beat == _loop.second;
  if (reached_circle_end) {
    bool grow_circle = this->isRecording() && !this->_record_params.previous_member && !this->checkState(1, 1, 0);
    if (grow_circle) {
      _loop.second += _loop_length;
      if (this->_record_params.next_member) {
        _recording_member->_n_beats += _loop_length;
      } else {
        _recording_member->_n_beats += 1;
      }
    } else {
      bool reflect_to_circle_start =
        (this->_reflect && !(this->isRecording() && !_record_params.next_member)) ||
        (_circle.atEnd(_circle_position) && !this->isRecording());
      if (reflect_to_circle_start) {
        _read_antipop_filter.trigger();
        _write_antipop_filter.trigger();
        _circle_position.beat = _loop.first;
        if (_options.create_new_member_on_reflect && this->isRecording()) {
          this->endRecording();
          _recording_member = this->newRecording();
        }
      } else { // shift circle
        unsigned int circle_shift = _loop.second - _loop.first;
        _loop.first = _loop.second;
        _loop.second += circle_shift;
      }
    }
  }

  bool reached_recording_end = this->isRecording() && _recording_member->_start_beat + _recording_member->_n_beats <= _circle_position.beat;
  if (reached_recording_end) {
    bool create_new_dub = this->checkState(1, 0, 1);
    if (create_new_dub) {
      this->endRecording();
      _recording_member = this->newRecording();
    } else if (!_record_params.previous_member || !_record_params.next_member) {
      _recording_member->_n_beats++;
    }
  }
}

inline PhaseAnalyzer::PhaseEvent Engine::advanceCirclePosition() {
  float internal_phase = _phase_oscillator.step(_sample_time);
  _circle_position.phase = _use_ext_phase ? _ext_phase : internal_phase;

  PhaseAnalyzer::PhaseEvent phase_event = _phase_analyzer.process(_circle_position.phase, _sample_time);

  unsigned int new_beat = _circle_position.beat;
  if (phase_event == PhaseAnalyzer::PhaseEvent::BACKWARD && 1 <= _circle_position.beat) {
    new_beat = _circle_position.beat - 1;
  } else if (phase_event == PhaseAnalyzer::PhaseEvent::FORWARD) {
    new_beat = _circle_position.beat + 1;
  }

  if (phase_event == PhaseAnalyzer::PhaseEvent::DISCONTINUITY && _options.use_antipop) {
    _read_antipop_filter.trigger();
  }

  _circle_position.beat = new_beat;

  return phase_event;
}

void Engine::step() {
  if (this->phaseDefined()) {
    PhaseAnalyzer::PhaseEvent phase_event = this->advanceCirclePosition();
    if (phase_event == PhaseAnalyzer::PhaseEvent::FORWARD || phase_event == PhaseAnalyzer::PhaseEvent::BACKWARD) {
      this->handleBeatChange(phase_event);
    }
  }

  if (!this->isRecording() && _record_params.active()) {
    _recording_member = this->newRecording();
    _write_antipop_filter.trigger();
  } else if (this->isRecording() && !_record_params.active()) {
    // if the capture button is used, assume the recording afterwards is unnecessary
    if (_used_window_capture_button) {
      Member *unnecessary_recording = _recording_member;
      _recording_member = nullptr;
      delete unnecessary_recording;
      _used_window_capture_button = false;
    } else {
      this->endRecording();
    }
    this->resetEngineMode();
  }

  if (this->isRecording()) {
    float in = _write_antipop_filter.process(_record_params.readIn());
    _recording_member->write(_circle_position, in, _record_params.love, this->phaseDefined());
  }
}
