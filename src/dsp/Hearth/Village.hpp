#pragma once

#include "Voice.hpp"
#include "Movement.hpp"
#include "Group.hpp"
#include "definitions.hpp"
#include "util/math.hpp"

#include <vector>

namespace kokopellivcv {
namespace dsp {
namespace hearth {

struct Village {
  std::vector<Group*> groups;
  Group* observed_sun = nullptr;

  std::vector<Voice*> voices;
  Voice *new_voice = nullptr;

  Outputs out;

  Village() {
    Group *first_group = new Group();
    this->groups.push_back(first_group);
    this->observed_sun = first_group;
    this->new_voice = new Voice(this->observed_sun);
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

} // namespace hearth
} // namespace dsp
} // namespace kokopellivcv
