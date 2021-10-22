#include "Engine.hpp"

using namespace tribalinterfaces::dsp::circle;
using namespace tribalinterfaces::dsp;

bool Engine::isRecording() {
  return _recording_layer != nullptr;
}

void Engine::setFixBounds(bool fix_bounds) {
  _record_params.fix_bounds = fix_bounds;
}

void Engine::loop() {
  if (this->isRecording()) {
    if (_record_params.fix_bounds) {
      this->endRecording(true, false);
    } else {
      _record_params.fix_bounds = true;
      this->endRecording(true, true);
    }
  } else {
    skipToActiveLayer();
  }
}
