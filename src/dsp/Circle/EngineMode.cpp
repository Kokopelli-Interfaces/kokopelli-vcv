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

void Engine::loopLongPress() {
  // TODO QUICK EDIT REC
  if (this->isRecording()) {
    this->endRecording(true);
    // TODO focus member
  }

  _loop_mode = LoopMode::Member;
}

void Engine::loop() {
  if (this->isRecording()) {
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

void Engine::resetEngineMode() {
  // _loop_mode = true;
  _record_params.record_on_inner_circle = true;
  _record_params.fix_bounds = false;
}
