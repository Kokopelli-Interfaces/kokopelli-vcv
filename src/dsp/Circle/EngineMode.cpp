#include "Engine.hpp"

using namespace tribalinterfaces::dsp::circle;
using namespace tribalinterfaces::dsp;

bool Engine::isRecording() {
  return _recording_layer != nullptr;
}

void Engine::setFixBounds(bool previous_member) {
  _record_params.previous_member = previous_member;
}

void Engine::setRecordOnInnerLoop(bool next_member) {
  if (this->isRecording()) {
    if (!_record_params.next_member && next_member)  {
      _circle.first = _timeline_position.beat;
      _circle.second = _timeline_position.beat + _loop_length;
    }

    this->endRecording();
    _record_params.next_member = next_member;
    _recording_layer = this->newRecording();
  }

  _record_params.next_member = next_member;
}

void Engine::setSkipBack(bool reflect) {
  if (this->isRecording()) {
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
