#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;
using namespace kokopellivcv::dsp;

void Engine::setCircleToMember(unsigned int member_i) {
  _circle.first = _timeline.members[member_i]->_start_beat;
  _circle.second =  _circle.first + _timeline.members[member_i]->_n_beats;
}

void Engine::skipToActiveMember() {
  if (0 < _timeline.members.size()) {
    setCircleToMember(_active_member_i);
    _timeline_position.beat = _circle.first;
  } else {
    _circle.first = 0;
    _circle.second = 1;
    _timeline_position.beat = 0;
  }

  _read_antipop_filter.trigger();

  if (isRecording()) {
    delete _recording_member;
    _recording_member = nullptr;
    _write_antipop_filter.trigger();
  }
}
