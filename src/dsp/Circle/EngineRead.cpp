#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;

float Engine::read() {
  float timeline_out = _timeline.read(_timeline_position, _recording_member, _record_params, _focused_member_i);

  if (_options.use_antipop) {
    timeline_out = _read_antipop_filter.process(timeline_out);
  }

  return kokopellivcv::dsp::sum(timeline_out, _record_params.in, _signal_type);
}

float Engine::readSelection() {
  float timeline_out = _timeline.readRawMembers(_timeline_position, _selected_members_idx);
  return timeline_out;
}

float Engine::readFocusedMember() {
  return _timeline.focused_member_out;
}
