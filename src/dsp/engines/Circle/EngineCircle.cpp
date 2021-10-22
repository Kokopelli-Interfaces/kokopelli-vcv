#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;
using namespace kokopellivcv::dsp;

// TODO REMOVE unnecessary circle first second updates if updateCirclePeriod

// unsigned int Engine::updateCirclePeriod() {
// // TODO depends on active members which are looping at current timeline beat
// }

void Engine::skipToActiveMember() {
  if (_timeline.members.size() == 0) {
    _group_loop.first = 0;
    _group_loop.second = 1;
    _group_loop_length = 1;
    _timeline_position.beat = 0;
    return;
  }

  // FIXME
  _group_loop.first = _timeline.members[_focused_member_i]->_start.beat;
  _group_loop.second =  _group_loop.first + _timeline.members[_focused_member_i]->_n_beats;
  _group_loop_length = _group_loop.second - _group_loop.first;

  _timeline_position.beat = _group_loop.first;
  _read_antipop_filter.trigger();
  _write_antipop_filter.trigger();

  // TODO
  // updateCirclePeriod
}
