#include "Engine.hpp"

using namespace myrisa::dsp::gko;
using namespace myrisa::dsp;

bool Engine::isRecording() {
  return _recording_layer != nullptr;
}

void Engine::setFixBounds(bool fix_bounds) {
  _record_params.fix_bounds = fix_bounds;
}

void Engine::setRecordOnInnerLoop(bool record_on_inner_circle) {
  if (this->isRecording()) {
    this->endRecording();
    _record_params.record_on_inner_circle = record_on_inner_circle;
    _recording_layer = this->newRecording();
    // _write_antipop_filter.trigger();
  }

  _record_params.record_on_inner_circle = record_on_inner_circle;
}

void Engine::setSkipBack(bool skip_back) {
  _skip_back = skip_back;
}

void Engine::resetEngineMode() {
  _skip_back = true;
  _record_params.record_on_inner_circle = true;
  _record_params.fix_bounds = true;
}
