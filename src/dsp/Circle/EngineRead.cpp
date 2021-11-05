#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;

float Engine::readAll() {
  float timeline_out = _timeline.read(_timeline_position);

  // FIXME assumes all selected
  timeline_out = timeline_out * (1 - _inputs.love);

  if (_options.use_antipop) {
    timeline_out = _read_antipop_filter.process(timeline_out);
  }

  return kokopellivcv::dsp::sum(timeline_out, _inputs.in, _signal_type);
}

// FIXME, only established
float Engine::readEstablished() {
  float timeline_out = _timeline.read(_timeline_position);
  return timeline_out;
}
