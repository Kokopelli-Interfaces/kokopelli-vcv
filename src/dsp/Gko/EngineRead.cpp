#include "Engine.hpp"

using namespace myrisa::dsp::gko;

// TODO just read rendered timeline
float Engine::read() {
  return _timeline.read(_timeline_position, _recording, _record_params);
}
