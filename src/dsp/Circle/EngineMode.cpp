#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;
using namespace kokopellivcv::dsp;

bool Engine::isRecording() {
  return _recording_member != nullptr;
}

void Engine::setFixBounds(bool fix_bounds) {
  _record_params.fix_bounds = fix_bounds;
}

void Engine::setRecordOnInnerLoop(bool record_on_inner_circle) {
  _record_params.record_on_inner_circle = record_on_inner_circle;
}

void Engine::repeat() {
  if (this->isRecording()) {
    this->endRecording(true);
  } else {
    _skip_back = !_skip_back;
  }
}

void Engine::resetEngineMode() {
  // _skip_back = true;
  _record_params.record_on_inner_circle = true;
  _record_params.fix_bounds = false;
}
