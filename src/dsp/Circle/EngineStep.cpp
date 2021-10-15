#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;
using namespace kokopellivcv::dsp;

inline bool Engine::phaseDefined() {
  return _use_ext_phase || _phase_oscillator.isSet();
}

// bool Engine::checkState(int skip_back, int fix_bounds, int record_on_inner_circle) {

void Engine::endRecording(bool loop_recording) {
  assert(isRecording());
  assert(_recording_member->_n_beats != 0);

  printf("- RECORDING END\n");

  if (!_phase_oscillator.isSet()) {
    if (_use_ext_phase && _phase_analyzer.getDivisionPeriod() != 0) {
      _phase_oscillator.setFrequency(1 / _phase_analyzer.getDivisionPeriod());
    } else {
      float recording_time = _recording_member->_in->_samples_per_beat * _sample_time;
      _phase_oscillator.setFrequency(1 / recording_time);
    }
    printf("-- phase oscillator set with frequency: %f, sample time is: %f\n", _phase_oscillator.getFrequency(), _sample_time);
  }

  if (loop_recording) {
    printf("-- group loop length: before %d -> after ", _group_loop_length);
    _recording_member->setLoop(true);

    unsigned int initial_group_loop_length = _group_loop_length;
    while (_group_loop_length < _recording_member->_n_beats) {
      _group_loop.second += initial_group_loop_length;
      _group_loop_length = _group_loop.second - _group_loop.first;
    }

    printf("%d\n", _group_loop_length);
    _recording_member->fitToLength(_group_loop_length);
  } else {
    assert(!_recording_member->isLooping());
  }

  _timeline.members.push_back(_recording_member);
  _timeline._last_calculated_attenuation.resize(_timeline.members.size());
  _timeline._current_attenuation.resize(_timeline.members.size());

  unsigned int member_i = _timeline.members.size() - 1;

  if (_select_new_members) {
    _selected_members_idx.push_back(member_i);
  }

  if (_new_member_active) {
    _active_member_i = member_i;
  }

  printf("-- start_beat %d n_beats %d  loop %d samples_per_beat %d member_i %d\n", _recording_member->_start.beat, _recording_member->_n_beats,  _recording_member->isLooping(), _recording_member->_in->_samples_per_beat, member_i);

  _recording_member = nullptr;
}

CircleMember* Engine::newRecording() {
  assert(_record_params.active());
  assert(_recording_member == nullptr);

  TimePosition start;
  start = _timeline_position;

  unsigned int n_beats = 1;
  // TODO ???
  // if (this->_record_params.fix_bounds && this->_record_params.record_on_inner_circle) {
  if (_loop_mode == LoopMode::Group) {
    start.beat = _group_loop.first;
    start.phase = 0.f; // FIXME circle bounds should have phase
    n_beats = _group_loop_length;
  }

  // bool shift_group_loop = !this->_record_params.fix_bounds;
  // if (shift_group_loop) {
  //   _group_loop.first = _timeline_position.beat;
  //   _group_loop.second = _group_loop.first + _group_loop_length;
  // }

  // if (this->_record_params.record_on_inner_circle) {
  //   if (this->_record_params.fix_bounds) {
  //     if (0 < _timeline.members.size()) {
  //       if (_timeline.members[_active_member_i]->_loop) {
  //         n_beats = _timeline.members[_active_member_i]->_n_beats;
  //       } else {
  //         n_beats = _group_loop_length;
  //       }
  //     }
  //   } else {
  //     n_beats = _timeline_position.beat - _group_loop.first + 1;
  //   }
  // }

  int samples_per_beat = 0;
  if (phaseDefined()) {
    if (_use_ext_phase) {
      samples_per_beat = _phase_analyzer.getSamplesPerBeat(_sample_time);
    } else if (_phase_oscillator.isSet()) {
      float beat_period = 1 / _phase_oscillator.getFrequency();
      samples_per_beat = floor(beat_period / _sample_time);
    }
  }

  CircleMember* recording_member = new CircleMember(_timeline_position, n_beats, _selected_members_idx, _signal_type, samples_per_beat);

  // if (this->_record_params.record_on_inner_circle) {
  //   recording_member->_loop = true;
  // }

  printf("Recording Activate:\n");
  printf("-- start_beat %d n_beats %d loop %d samples per beat %d active circle member %d\n", recording_member->_start.beat, recording_member->_n_beats, recording_member->_loop, recording_member->_in->_samples_per_beat, _active_member_i);

  return recording_member;
}

inline void Engine::handleBeatChange(PhaseAnalyzer::PhaseEvent event) {
  assert(phaseDefined());

  bool reached_group_loop_end = _timeline_position.beat == _group_loop.second;
  if (reached_group_loop_end) {
    if (_loop_mode == LoopMode::Group) {
      _read_antipop_filter.trigger();
      _write_antipop_filter.trigger();
      _timeline_position.beat = _group_loop.first;
    } else if (_loop_mode == LoopMode::None) {
      unsigned int group_loop_shift = _group_loop.second - _group_loop.first;
      _group_loop.first = _group_loop.second;
      _group_loop.second += group_loop_shift;
    }
  }

  //   bool grow_circle = this->isRecording() && !this->_record_params.fix_bounds && !this->checkState(1, 1, 0);
  //   if (grow_circle) {
  //     _group_loop.second += _group_loop_length;
  //     if (this->_record_params.record_on_inner_circle) {
  //       _recording_member->_n_beats += _group_loop_length;
  //     } else {
  //       _recording_member->_n_beats += 1;
  //     }
  //   } else {
  //     bool skip_back_to_circle_start = (this->_loop_mode && !(this->isRecording() && !_record_params.record_on_inner_circle));
  //     if (skip_back_to_circle_start) {
  //       _read_antipop_filter.trigger();
  //       _write_antipop_filter.trigger();
  //       _timeline_position.beat = _group_loop.first;
  //     } else { // shift circle
  //       unsigned int group_loop_shift = _group_loop.second - _group_loop.first;
  //       _group_loop.first = _group_loop.second;
  //       _group_loop.second += group_loop_shift;
  //     }
  //   }
  // }

  // TODO do in recording, not here
  bool reached_recording_end = this->isRecording() && _recording_member->_start.beat + _recording_member->_n_beats <= _timeline_position.beat;
  if (reached_recording_end) {
    _recording_member->_n_beats++;
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
    _recording_member = this->newRecording();
    _write_antipop_filter.trigger();
  } else if (this->isRecording() && !_record_params.active()) {
    this->endRecording(false);
    this->resetEngineMode();
  }

  if (this->isRecording()) {
    float in = _write_antipop_filter.process(_record_params.readIn());
    _recording_member->write(_timeline_position, in, _record_params.new_love, this->phaseDefined());
  }
}
