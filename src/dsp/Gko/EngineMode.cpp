#include "Engine.hpp"

using namespace myrisa::dsp::gko;
using namespace myrisa::dsp;

bool Engine::isRecording() {
  return _recording_layer != nullptr;
}

void Engine::setRecordMode(RecordParams::Mode mode) {
  _record_params.mode = mode;
}

void Engine::setRecordOnOuterLoop(bool record_on_outer_loop) {
  if (this->isRecording()) {
    this->endRecording();
    _record_params.record_on_outer_loop = record_on_outer_loop;
    _recording_layer = this->newRecording();
    // _write_antipop_filter.trigger();
  }

  _record_params.record_on_outer_loop = record_on_outer_loop;
}

void Engine::setSkipBack(bool skip_back) {
  _skip_back = skip_back;
}

void Engine::resetEngineMode() {
  this->setSkipBack(true);
  this->setRecordOnOuterLoop(false);
  this->setRecordMode(RecordParams::Mode::DUB);
}
