#include "Group.hpp"

using namespace kokopellivcv::dsp::hearth;
using namespace kokopellivcv::dsp;

float Group::getMostRecentMovementPhase() {
  return 1.f;
}

unsigned int Group::getMostRecentMovement(int offset) {
  if (_movements.empty()) {
    return 0;
  }

  int index = _current_movement_i + offset;
  while (index < 0) {
    index += _movements.size();
  }
  return (index % _movements.size()) + 1;
}

void Group::addVoiceToMovements(Voice *voice) {
  if (_movements.empty()) {
    _movements.push_back(0.f);
  }

  _voice_i_to_movement_i.push_back(_current_movement_i);
  assert(_voice_i_to_movement_i.size() == _voices_in_group.size());
}
