#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;

void Engine::next() {
  if (isRecording()) {
    if (_record_params.fix_bounds) {
      this->endRecording(true, false);
    } else {
      endRecording(false, false);
    }
  } else {
    nextMember();
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
  if (this->isRecording()) {
    this->endRecording(false, false);
    forget();
  } else {
    prevMember();
  }
}

void Engine::toggleMemberMode() {
  unsigned int member_i = _active_member_i;

  if (isRecording()) {
    prev();
    member_i = _timeline.members.size()-1;
  }

  if (!_member_mode) {
    _member_mode = true;
    _selected_members_idx_before_member_mode = _selected_members_idx;
    _circle_before_member_mode = _circle;

    setCircleToMember(member_i);
    _timeline_position.beat = _circle.first;
    _read_antipop_filter.trigger();
    soloSelectMember(_active_member_i);
  } else {
    _selected_members_idx = _selected_members_idx_before_member_mode;
    _circle = _circle_before_member_mode;
    _member_mode = false;
  }
}
