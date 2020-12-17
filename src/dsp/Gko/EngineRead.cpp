#include "Engine.hpp"

using namespace myrisa::dsp::gko;

float Engine::read() {
  float timeline_out = _timeline.read(_timeline_position, _recording_layer, _record_params);
  if (_options.use_antipop) {
    timeline_out = _read_antipop_filter.process(timeline_out);
  }

  return timeline_out;
}
