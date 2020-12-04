#include "Engine.hpp"

using namespace myrisa::dsp::gko;

// TODO just read rendered timeline
float Engine::read() {
  float timeline_out = _timeline.read(_timeline_position, _recording_layer, _record_params);
  // return timeline_out;
  if (_options.use_antipop) {
    timeline_out = _antipop_filter.process(timeline_out);
  }

  return timeline_out;
}
