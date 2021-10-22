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
    this->endRecording(true);
  }
}

void Engine::resetEngineMode() {
  _record_params.fix_bounds = true;
}
