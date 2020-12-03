#include "Engine.hpp"

using namespace myrisa::dsp::gko;

// TODO just read rendered timeline
float Engine::read() {
  float timeline_out = _timeline.read(_timeline_position, _recording_layer, _record_params);
  // return timeline_out;
  return _antipop_filter.process(timeline_out);
}
