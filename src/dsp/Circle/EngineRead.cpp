#include "Engine.hpp"

using namespace kokopelli::dsp::circle;

float Engine::read() {
  float circle_out = _circle.hear(_circle_position, _recording_member, _record_params, _active_member_i);

  if (_options.use_antipop) {
    circle_out = _read_antipop_filter.process(circle_out);
  }

  return kokopelli::dsp::sum(circle_out, _record_params.readIn(), _signal_type);
}
