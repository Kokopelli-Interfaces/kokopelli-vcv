#include "Group.hpp"

using namespace kokopellivcv::dsp::hearth;
using namespace kokopellivcv::dsp;

float Group::getMostRecentMovementPhase() {
  return 1.f;
}

unsigned int Group::getMostRecentMovement(int offset) {
  if (this->movements.empty()) {
    return 0;
  }

  int index = this->movement_i + offset;
  while (index < 0) {
    index += this->movements.size();
  }
  return (index % this->movements.size()) + 1;
}
