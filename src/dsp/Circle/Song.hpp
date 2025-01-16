#pragma once

#include "Cycle.hpp"
#include "Group.hpp"
#include "definitions.hpp"
#include "util/math.hpp"

#include <vector>

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct Song {
  std::string name = "My Song";

  std::vector<Group*> groups;
  Group* observed_sun = nullptr;

  std::vector<Cycle*> cycles;

  Cycle *new_cycle = nullptr;

  Time playhead = 0.f;

  Outputs out;

  Song() {
    this->groups.push_back(new Group());
    this->observed_sun = groups[0];
    int first_movement = 0;
    Time offset_in_group = 0.f;
    this->new_cycle = new Cycle(this->observed_sun, offset_in_group, first_movement);
  }

  void clearEmptyGroups() {
    for (int i = groups.size()-1; 0 <= i; i--) {
      if (groups[i]->cycles_in_group.size() == 0) {
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
