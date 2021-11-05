#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;
using namespace kokopellivcv::dsp;

int Engine::getMostRecentLoopLength() {
  for (int member_i = _timeline.members.size()-1; member_i >= 0; member_i--) {
    if (_timeline.members[member_i]->_loop) {
      return _timeline.members[member_i]->_n_beats;
    }
  }

  return -1;
}

void Engine::deleteMember(unsigned int member_i) {
  if (member_i < _timeline.members.size()) {
    _timeline.members.erase(_timeline.members.begin()+member_i);
    if (_focused_member_i == member_i && _focused_member_i != 0) {
      _focused_member_i--;
    }
  }

  if (_timeline.members.size() == 0) {
    _phase_oscillator.reset(0.f);
  }
}

// TODO FIXME
void Engine::nextGroup() {
  for (int member_i = _timeline.members.size()-1; member_i >= 0; member_i--) {
    _timeline.members.erase(_timeline.members.begin()+member_i);
  }

  _phase_oscillator.reset(0.f);
}

bool Engine::isRecording() {
  return _new_member != nullptr;
}

void Engine::toggleTuneToFrequencyOfEstablished() {
  if (_love_direction != LoveDirection::ESTABLISHED &&
    !_tune_to_frequency_of_established) {
    this->endRecording(true, true);
  }

  _tune_to_frequency_of_established = !_tune_to_frequency_of_established;
}

void Engine::forward() {
  if (_love_direction == LoveDirection::NEW) {
    // TODO new group
  }

  if (_tune_to_frequency_of_established) {
    if (_love_direction == LoveDirection::ESTABLISHED) {
      // TODO next section via insertion
      // AI -> AII.
      unsigned int circle_n_beats = _circle.second - _circle.first;
      _circle.first = _timeline_position.beat;
      _circle.second = _timeline_position.beat + circle_n_beats;
    } else {
      this->endRecording(true, false);
    }
  } else {
    if (_love_direction == LoveDirection::ESTABLISHED) {
      // A -> B
      this->endRecording(false, false);
    } else {
      // I -> II.
      this->endRecording(false, false);
    }
  }
}

// TODO
void Engine::backward() {
  if (_love_direction == LoveDirection::ESTABLISHED) {
    // enter REMEMBER mode -> change NEW to something
    if (_tune_to_frequency_of_established) {
      if (_member_mode) {
        // ??? cycle focused_member (displays on NEW in purple)
      } else {
        // cycle focused_group (displays on NEW in purple)
        // use to  enter
      }
    } else {
      // backward one section, prevSection();
    }
  } else if (_love_direction == LoveDirection::EMERGENCE) {
      this->endRecording(false, false);
      this->forget();
  } else {
      // cycle focused_group, then, pressing next
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

// FIXME
void Engine::toggleMemberMode() {
  // TODO set established to member, right shows layers

  // unsigned int member_i = _focused_member_i;

  // if (isRecording()) {
  //   member_i = _timeline.members.size()-1;
  // }

  // if (!_member_mode) {
  //   _member_mode = true;
  //   _selected_members_idx_before_member_mode = _selected_members_idx;
  //   _circle_before_member_mode = _circle;

  //   setCircleToMember(member_i);
  //   _timeline_position.beat = _circle.first;
  //   _read_antipop_filter.trigger();
  // } else {
  //   _selected_members_idx = _selected_members_idx_before_member_mode;
  //   _circle = _circle_before_member_mode;
  //   _member_mode = false;
  // }
}
