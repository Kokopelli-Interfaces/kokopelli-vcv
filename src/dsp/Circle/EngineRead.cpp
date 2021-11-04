#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;

float Engine::read() {
  float timeline_out = _timeline.read(_timeline_position, _focused_member_i);

  // FIXME assumes all selected
  if (_record_params.active()) {
    timeline_out = timeline_out * (1 - _record_params.love);
  }

  if (_options.use_antipop) {
    timeline_out = _read_antipop_filter.process(timeline_out);
  }

  return kokopellivcv::dsp::sum(timeline_out, _record_params.in, _signal_type);
}

float Engine::readWithoutRecordingMember() {
  float timeline_out = _timeline.read(_timeline_position, _focused_member_i);
  return timeline_out;
}

float Engine::readFocusedMember() {
  return _timeline.focused_member_out;
}
