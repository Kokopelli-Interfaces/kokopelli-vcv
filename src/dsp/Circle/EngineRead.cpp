#include "Engine.hpp"

using namespace tribalinterfaces::dsp::circle;

float Engine::read() {
  float circle_out = _circle.read(_circle_position, _recording_member, _record_params, _active_member_i);

  if (_options.use_antipop) {
    circle_out = _read_antipop_filter.process(circle_out);
  }

  return tribalinterfaces::dsp::sum(circle_out, _record_params.readIn(), _signal_type);
}

float Engine::readSelection() {
  float circle_out = _circle.readRawMembers(_circle_position, _selected_members_idx);
  return circle_out;
}

float Engine::readActiveMember() {
  return _circle.active_member_out;
}
