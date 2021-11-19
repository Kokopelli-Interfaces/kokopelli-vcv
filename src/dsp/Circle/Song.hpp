#pragma once

#include "Cycle.hpp"
#include "Movement.hpp"
#include "Group.hpp"
#include "definitions.hpp"
#include "util/math.hpp"

#include <vector>

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct Song {
  std::string name = "my song";

  std::vector<Group*> groups;
  Group* established;

  std::vector<Cycle*> cycles;
  // TODO move to inactive if pre-entrance movement
  // std::vector<Cycle*> inactive_cycles;

  Cycle *new_cycle = nullptr;

  Time playhead = 0.f;

  Movement *current_movement = nullptr;
  Movement* start_movement = nullptr;

  Outputs out;

  Song() {
    this->current_movement = new Movement();
    this->current_movement->group_start_movement = this->current_movement;
    this->start_movement = this->current_movement;
    Time start = 0.f;
    this->groups.push_back(new Group());
    this->established = groups[0];
    this->new_cycle = new Cycle(start, this->current_movement, this->established);
  }

  void clearEmptyGroups() {
    for (int i = groups.size()-1; 0 <= i; i--) {
      if (groups[i]->cycles_in_group.size() == 0) {
        if (groups[i]->parent_group == nullptr) {
          continue;
        }

        if (groups[i] == established && groups[i]->parent_group) {
          established = groups[i]->parent_group;
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
