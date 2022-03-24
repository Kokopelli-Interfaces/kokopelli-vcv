#pragma once

#include "definitions.hpp"
#include "Group.hpp"

#include <vector>

namespace kokopellivcv {
namespace dsp {
namespace hearth {

struct Conductor {
  Group* focus_group = nullptr;
  bool is_choosing_next_group = false;
  Group* parent_of_next_focus_group = nullptr;
  std::vector<Group*> potential_next_focus_groups;
  int selected_group_i_in_potential = -1;

  inline bool checkIfCanEnterFocusedSubgroup() {
    assert(this->is_choosing_next_group);
    Group* focused_subgroup = this->potential_next_focus_groups[this->selected_group_i_in_potential];
    bool can_enter =  1 < focused_subgroup->voices.size();
    return can_enter;
  }
};

} // namespace hearth
} // namespace dsp
} // namespace kokopellivcv
