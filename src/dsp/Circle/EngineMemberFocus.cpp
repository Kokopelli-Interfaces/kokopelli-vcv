#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;

void Engine::nextMember() {
  if (this->isRecording()) {
    this->endRecording(false);

    // TODO put member in NEW circle
    _group_loop.first = _timeline_position.beat;
    _group_loop.second = _group_loop.first + 1;
    _group_loop_length = 1;
  } else {
    // TODO seek previous member from current beat, may not be active
    if (_active_member_i == _timeline.members.size()-1) {
      _active_member_i = 0;
    } else {
      _active_member_i++;
    }

    skipToActiveMember();
  }
}

void Engine::prevMember() {
  if (this->isRecording()) {
    // TODO create sub-circle
  } else {
    // TODO prev member
    if (_active_member_i == 0) {
      _active_member_i = _timeline.members.size()-1;
    } else {
      _active_member_i--;
    }

    skipToActiveMember();
  }
}
