#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;

bool Engine::isRecording() {
  return _recording_member != nullptr;
}

void Engine::toggleTuneToGroupFrequency() {
  if (!_fully_love_group && !_record_params.tuned_to_group_frequency) {
    this->endRecording(true, true);
  }

  _record_params.tuned_to_group_frequency = !_record_params.tuned_to_group_frequency;
}


void Engine::next() {
  if (_record_params.tuned_to_group_frequency) {
    if (_fully_love_group) {
      // I -> II.
      unsigned int circle_n_beats = _circle.second - _circle.first;
      _circle.first = _timeline_position.beat;
      _circle.second = _timeline_position.beat + circle_n_beats;
    } else {
      this->endRecording(true, false);
    }
  } else {
    if (_fully_love_group) {
      // A -> B
      this->endRecording(false, false);
    } else {
      // I -> II.
      this->endRecording(false, false);
    }
  }
}

void Engine::forget() {
  int last_i = _timeline.members.size()-1;
  if (0 <= last_i) {
    _circle = _timeline.members[last_i]->_circle_before;
    unsigned int circle_length = _circle.second - _circle.first;

    while (_circle.second <= _timeline_position.beat) {
      _timeline_position.beat -= circle_length;
    }

    this->deleteMember(last_i);
  }
}

void Engine::prev() {
  // unsigned int start_beat = _recording_member->_start_beat;
  this->endRecording(false, false);
  this->forget();
  // _timeline_position.beat = start_beat;
}

void Engine::toggleMemberMode() {
  unsigned int member_i = _focused_member_i;

  if (isRecording()) {
    member_i = _timeline.members.size()-1;
  }

  if (!_member_mode) {
    _member_mode = true;
    _selected_members_idx_before_member_mode = _selected_members_idx;
    _circle_before_member_mode = _circle;

    setCircleToMember(member_i);
    _timeline_position.beat = _circle.first;
    _read_antipop_filter.trigger();
    soloSelectMember(_focused_member_i);
  } else {
    _selected_members_idx = _selected_members_idx_before_member_mode;
    _circle = _circle_before_member_mode;
    _member_mode = false;
  }
}
