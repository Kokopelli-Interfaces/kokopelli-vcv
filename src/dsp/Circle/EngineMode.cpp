#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;
using namespace kokopellivcv::dsp;

bool Engine::isRecording() {
  return _recording_member != nullptr;
}

void Engine::toggleFixBounds() {
  if (isRecording() && !_record_params.fix_bounds) {
    this->endRecording(true, true);
  }

  _record_params.fix_bounds = !_record_params.fix_bounds;
}
