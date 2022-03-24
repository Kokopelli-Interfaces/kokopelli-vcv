#pragma once

#include "Voice.hpp"
#include "Group.hpp"
#include "definitions.hpp"

#include <vector>

namespace kokopellivcv {
namespace dsp {
namespace hearth {

class GroupMovementChanger {
public:
  static inline void addVoiceToMovements(Group *group, Voice *voice) {
    if (group->movements.empty()) {
      group->movements.push_back(0.f);
    }

    group->voice_i_to_movement_i.push_back(group->current_movement_i);
    assert(group->voice_i_to_movement_i.size() == group->voices.size());
  }
};

} // namespace hearth
} // namespace dsp
} // namespace kokopellivcv
