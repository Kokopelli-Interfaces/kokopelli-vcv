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

void Engine::fitMemberIntoCircle(Member* member) {
  unsigned int circle_n_beats = _circle.second - _circle.first;
  if (member->_n_beats == circle_n_beats) {
    return;
  }

  if (member->_n_beats < circle_n_beats) {
    member->_n_beats = findNiceNumberAroundFirstThatFitsIntoSecond(member->_n_beats, circle_n_beats);
  } else {
    unsigned int new_circle_n_beats = circle_n_beats;
    while (new_circle_n_beats < member->_n_beats) {
      new_circle_n_beats += circle_n_beats;
    }
    member->_n_beats = new_circle_n_beats;
    _circle.second = _circle.first + new_circle_n_beats;
  }
}

void Engine::endRecording(bool loop, bool create_new_circle) {
  assert(isRecording());
  assert(_new_member->_n_beats != 0);

  if (!_phase_oscillator.isSet()) {
    if (_use_ext_phase && _phase_analyzer.getDivisionPeriod() != 0) {
      _phase_oscillator.setFrequency(1 / _phase_analyzer.getDivisionPeriod());
    } else {
      float recording_time = _new_member->_in->_samples_per_beat * _sample_time;
      _phase_oscillator.setFrequency(1 / recording_time);
    }
    // printf("-- phase oscillator set with frequency: %f, sample time is: %f\n", _phase_oscillator.getFrequency(), _sample_time);
  }

  _new_member->_loop = loop;
  if (loop) {
    if (create_new_circle) {
      unsigned int new_circle_n_beats = _new_member->_n_beats;
      _circle.second = _circle.first + new_circle_n_beats;
    } else {
      fitMemberIntoCircle(_new_member);
    }
  }

  _timeline.members.push_back(_new_member);
  _timeline._next_love.resize(_timeline.members.size());

  unsigned int member_i = _timeline.members.size() - 1;

  _selected_members_idx.push_back(member_i);
  _focused_member_i = member_i;


  // printf("- rec end\n");
  // printf("-- start_beat %d n_beats %d  loop %d samples_per_beat %d member_i %d\n", _new_member->_start_beat, _new_member->_n_beats,  _new_member->_loop, _new_member->_in->_samples_per_beat, member_i);
  _new_member = nullptr;
}

Member* Engine::newRecording() {
  if (_new_member) {
    delete _new_member;
    _new_member = nullptr;
  }

  assert(_new_member == nullptr);

  unsigned int start_beat = _timeline_position.beat;
  unsigned int n_beats = 1;

  unsigned int circle_n_beats = _circle.second - _circle.first;
  if (_tune_to_frequency_of_established) {
    start_beat = _circle.first;
    n_beats = circle_n_beats;
  }

  bool shift_circle = !_tune_to_frequency_of_established;
  if (shift_circle) {
    _circle.first = _timeline_position.beat;
    _circle.second = _timeline_position.beat + circle_n_beats;
  }

  if (_tune_to_frequency_of_established) {
    int recent_loop_length = getMostRecentLoopLength();
    if (recent_loop_length != -1) {
      n_beats = recent_loop_length;
    } else {
      n_beats = circle_n_beats;
    }
  }

  // else {
  //   n_beats = _timeline_position.beat - _circle.first + 1;
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

  Member* recording_member = new Member(start_beat, n_beats, _selected_members_idx, _signal_type, samples_per_beat);

  recording_member->_circle_before = _circle;

  // printf("Recording Activate:\n");
  // printf("-- start_beat %d n_beats %d loop %d samples per beat %d focused member %d\n", recording_member->_start_beat, recording_member->_n_beats, recording_member->_loop, recording_member->_in->_samples_per_beat, _focused_member_i);

  return recording_member;
}

inline void Engine::handleBeatChange(PhaseAnalyzer::PhaseEvent event) {
  assert(phaseDefined());

  bool reached_circle_end = _timeline_position.beat == _circle.second;

  if (reached_circle_end) {
    bool shift_circle = this->isRecording() && !_tune_to_frequency_of_established;
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

  bool reached_recording_end = this->isRecording() && _new_member->_start_beat + _new_member->_n_beats <= _timeline_position.beat;
  if (reached_recording_end && !(_tune_to_frequency_of_established && _love_direction == LoveDirection::ESTABLISHED)) {
    _new_member->_n_beats++;
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

  if (!this->isRecording()) {
    _new_member = this->newRecording();
    _write_antipop_filter.trigger();
  }

  float in = _write_antipop_filter.process(_inputs.in);
  _new_member->write(_timeline_position, in, _inputs.love, this->phaseDefined());

  LoveDirection new_direction = Inputs::getLoveDirection(_inputs.love);
  if (_love_direction != new_direction) {
    if (_love_direction == LoveDirection::ESTABLISHED) {
      _new_member = this->newRecording();
    } else if (new_direction == LoveDirection::ESTABLISHED) {
      this->endRecording(true, false);
    }
    _love_direction = new_direction;
  }
}
