#include "Engine.hpp"

using namespace tribalinterfaces::dsp::circle;
using namespace tribalinterfaces::dsp;

bool Engine::isLoving() {
  return _recording_member != nullptr;
}

void Engine::setFixBounds(bool previous_member) {
  _record_params.previous_member = previous_member;
}

void Engine::setRecordOnInnerLoop(bool next_member) {
  if (this->isLoving()) {
    if (!_record_params.next_member && next_member)  {
      _loop.first = _circle_position.beat;
      _loop.second = _circle_position.beat + _loop_length;
    }

    this->endRecording();
    _record_params.next_member = next_member;
    _recording_member = this->newRecording();
  }

  _record_params.next_member = next_member;
}

void Engine::setSkipBack(bool reflect) {
  if (this->isLoving()) {
    _used_window_capture_button = true;
    this->endRecording();
  } else {
    _reflect = reflect;
  }
}

void Engine::resetEngineMode() {
  _reflect = true;
  _record_params.next_member = true;
  _record_params.previous_member = true;
}
