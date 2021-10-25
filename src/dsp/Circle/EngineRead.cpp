#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;

float Engine::read() {
  float timeline_out = _timeline.read(_timeline_position, _recording_member, _record_params, _active_member_i);

  if (_options.use_antipop) {
    timeline_out = _read_antipop_filter.process(timeline_out);
  }

  return timeline_out;
}

float Engine::readSelection() {
  float timeline_out = _timeline.readRawMembers(_timeline_position, _selected_members_idx);
  return timeline_out;
}

float Engine::readActiveMember() {
  return _timeline.active_member_out;
}
