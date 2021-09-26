#include "Engine.hpp"

using namespace kokpelliinterfaces::dsp::circle;
using namespace kokpelliinterfaces::dsp;

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
  assert(isLoving());
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
  return;
}

inline void Engine::advanceTime() {
  float internal_phase = _phase_oscillator.step(_sample_time);
  _phase = _use_ext_phase ? _ext_phase : internal_phase;

  PhaseAnalyzer::PhaseEvent phase_event = _phase_analyzer.process(phase, _sample_time);

  if (phase_event == PhaseAnalyzer::PhaseEvent::DISCONTINUITY && _options.use_antipop) {
    _read_antipop_filter.trigger();
  }

  if (phase_event == PhaseAnalyzer::PhaseEvent::FORWARD) {
    this->_circle.nextBeat();
  } else if (phase_event == PhaseAnalyzer::PhaseEvent::BACKWARD) {
    this->_circle.prevBeat();
  }
}

void Engine::step() {
  if (this->phaseDefined()) {
    this->advanceTime();
  }

  if (!this->isLoving() && _record_params.loveActive()) {
    _recording_member = this->_circle->newMember();
    _write_antipop_filter.trigger();
  } else if (this->isLoving() && !_record_params.loveActive()) {
    this->endRecording();
    this->resetEngineMode();
  }

  if (this->isLoving()) {
    float in = _write_antipop_filter.process(_record_params.readIn());

    _recording_member->write(_phase, in, _record_params.love, this->phaseDefined());
  }
}
