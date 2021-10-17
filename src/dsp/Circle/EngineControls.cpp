#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;

void Engine::undo() {
  if (_state.loving) {
    endRecording(false);
  }

  this->deleteMember(_timeline.members.size()-1);
}

void Engine::next() {
  if (this->_state.loving) {
    this->endRecording(false);

    // TODO put member in NEW circle
    _group_loop.first = _timeline_position.beat;
    _group_loop.second = _group_loop.first + 1;
    _group_loop_length = 1;
  } else {
    // TODO seek previous member from current beat, may not be active
    if (_focused_member_i == _timeline.members.size()-1) {
      _focused_member_i = 0;
    } else {
      _focused_member_i++;
    }

    skipToActiveMember();
  }
}

void Engine::prev() {
  if (this->_state.loving) {
    // TODO create sub-circle
  } else {
    // TODO prev member
    if (_focused_member_i == 0) {
      _focused_member_i = _timeline.members.size()-1;
    } else {
      _focused_member_i--;
    }

    skipToActiveMember();
  }
}

void Engine::loopLongPress() {
  // TODO QUICK EDIT REC
  if (this->_state.loving) {
    this->endRecording(true);
    // TODO focus member
  }

  _loop_mode = LoopMode::Member;
}

void Engine::loop() {
  if (this->_state.loving) {
    this->endRecording(true);
    return;
  }

  switch (_loop_mode) {
  case LoopMode::Member:
    _loop_mode = LoopMode::Group;
    break;
  case LoopMode::Group:
    _loop_mode = LoopMode::None;
    break;
  case LoopMode::None:
    _loop_mode = LoopMode::Group;
    break;
  }

}
