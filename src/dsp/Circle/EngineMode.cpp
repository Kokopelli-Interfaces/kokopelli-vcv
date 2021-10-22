#include "Engine.hpp"

using namespace tribalinterfaces::dsp::circle;
using namespace tribalinterfaces::dsp;

bool Engine::isRecording() {
  return _recording_layer != nullptr;
}

void Engine::setFixBounds(bool fix_bounds) {
  _record_params.fix_bounds = fix_bounds;
}
