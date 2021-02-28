#include "Engine.hpp"

using namespace myrisa::dsp::gko;
using namespace myrisa::dsp;

bool Engine::isRecording() {
  return _recording_layer != nullptr;
}

void Engine::setRecordMode(RecordParams::Mode mode) {
  _record_params.mode = mode;
}

void Engine::setRecordTimeFrame(TimeFrame frame) {
  if (this->isRecording()) {
    this->endRecording();
    _record_params.time_frame = frame;
    _recording_layer = this->newRecording();
    // _write_antipop_filter.trigger();
  }

  _record_params.time_frame = frame;
}

void Engine::setReadTimeFrame(TimeFrame frame) {
  _read_time_frame = frame;
}
