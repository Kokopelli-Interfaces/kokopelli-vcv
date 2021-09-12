#include "Engine.hpp"

using namespace tribalinterfaces::dsp::circle;
using namespace tribalinterfaces::dsp;

bool Engine::isRecording() {
  return _recording_layer != nullptr;
}

void Engine::setFixBounds(bool fix_bounds) {
  _record_params.fix_bounds = fix_bounds;
}

void Engine::setRecordOnInnerLoop(bool record_on_inner_circle) {
  if (this->isRecording()) {
    if (!_record_params.record_on_inner_circle && record_on_inner_circle)  {
      _circle.first = _timeline_position.beat;
      _circle.second = _timeline_position.beat + _loop_length;
    }

    this->endRecording();
    _record_params.record_on_inner_circle = record_on_inner_circle;
    _recording_layer = this->newRecording();
  }

  _record_params.record_on_inner_circle = record_on_inner_circle;
}

void Engine::setSkipBack(bool skip_back) {
  if (this->isRecording()) {
    _used_window_capture_button = true;
    this->endRecording();
  } else {
    _skip_back = skip_back;
  }
}

void Engine::resetEngineMode() {
  _skip_back = true;
  _record_params.record_on_inner_circle = true;
  _record_params.fix_bounds = true;
}
