#include "Engine.hpp"

using namespace tribalinterfaces::dsp::circle;
using namespace tribalinterfaces::dsp;

void Engine::setCircleToActiveMember() {
  if (0 < _circle.members.size()) {
    _loop.first = _circle.members[_active_member_i]->_start_beat;
    _loop.second =  _loop.first + _circle.members[_active_member_i]->_n_beats;
    _circle_position.beat = _loop.first;
  } else {
    _loop.first = 0;
    _loop.second = 1;
    _circle_position.beat = 0;
  }

  _loop_length = _loop.second - _loop.first;

  _read_antipop_filter.trigger();
  _write_antipop_filter.trigger();
}
