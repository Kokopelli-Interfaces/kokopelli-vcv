#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;

float Engine::read() {
  float timeline_out = _timeline.read(_timeline_position, _recording_member, _params, _focused_member_i);

  if (_options.use_antipop) {
    timeline_out = _read_antipop_filter.process(timeline_out);
  }

  return timeline_out;
}
