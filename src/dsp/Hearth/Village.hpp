#pragma once

#include "Voice.hpp"
#include "Movement.hpp"
#include "Group.hpp"
#include "definitions.hpp"
#include "util/math.hpp"

#include <vector>

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct Village {
  std::vector<Group*> groups;
  Group* observed_sun = nullptr;

  std::vector<Voice*> voices;
  // TODO move to inactive if pre-entrance movement
  // std::vector<Voice*> inactive_voices;

  Voice *new_voice = nullptr;

  Time playhead = 0.0;

  Movement *current_movement = nullptr;
  Movement *start_movement = nullptr;

  Outputs out;

  Village() {
    this->current_movement = new Movement();
    this->current_movement->group_start_movement = this->current_movement;
    this->start_movement = this->current_movement;
    Time start = 0.f;
    Group *first_group = new Group();
    this->groups.push_back(first_group);
    this->observed_sun = first_group;
    this->new_voice = new Voice(start, this->current_movement, this->observed_sun);
  }

  void clearEmptyGroups() {
    for (int i = groups.size()-1; 0 <= i; i--) {
      if (groups[i]->voices_in_group.size() == 0) {
        if (groups[i]->parent_group == nullptr) {
          continue;
        }

        if (groups[i] == observed_sun && groups[i]->parent_group) {
          observed_sun = groups[i]->parent_group;
        }

        Group* delete_group = groups[i];
        groups.erase(groups.begin() + i);
        delete delete_group;
      }
    }
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
