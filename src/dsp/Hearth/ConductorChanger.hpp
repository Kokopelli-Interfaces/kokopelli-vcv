#pragma once

#include <vector>
#include <numeric> // std::iota

#include "rack.hpp"

#include "Conductor.hpp"
#include "Voice.hpp"
#include "Group.hpp"
#include "GroupChanger.hpp"
#include "definitions.hpp"

namespace kokopellivcv {
namespace dsp {
namespace hearth {

class ConductorChanger {
public:
  static inline std::vector<Group*> breakIntoPotentialNextFocusGroups(Group* parent) {
    std::vector<Group*> potential_next_focus_groups;

    for (unsigned int i = 0; i < parent->voices.size(); i++) {
      Voice* voice = parent->voices[i];
      bool create_subgroup = voice->immediate_group == parent;
      if (create_subgroup) {
        Group* subgroup = new Group();
        potential_next_focus_groups.push_back(subgroup);
        subgroup->parent_group = parent;
        subgroup->name = rack::string::f("%s/%d", parent->name.c_str(), i + 1);
        subgroup->movements.push_back(0.f);
        subgroup->voice_i_to_movement_i.push_back(0);

        GroupChanger::addExistingVoice(subgroup, voice);
        // voice->immediate_group = subgroup;
      } else {
        if (voice->immediate_group->parent_group == parent) {
          bool already_accounted_for = std::find(potential_next_focus_groups.begin(), potential_next_focus_groups.end(), voice->immediate_group) != potential_next_focus_groups.end();
          if (!already_accounted_for) {
            potential_next_focus_groups.push_back(voice->immediate_group);
          }
        }
      }
    }

    return potential_next_focus_groups;
  }

  static inline bool checkIfVoiceInGroupOneIsObservedByVoiceInGroupTwo(Group *one, Group *two) {
    if (!one) {
      return false;
    }

    if (one == two) {
      return true;
    }

    return checkIfVoiceInGroupOneIsObservedByVoiceInGroupTwo(one->parent_group, two);
  }

  static inline void tryEnterChooseNextGroupMode(Conductor *conductor, std::vector<Group*> &groups) {
    assert(!conductor->is_choosing_next_group);
    assert(conductor->focus_group);

    conductor->parent_of_next_focus_group = conductor->focus_group;
    if (conductor->parent_of_next_focus_group->voices.empty()) {
      return;
    }

    conductor->potential_next_focus_groups = breakIntoPotentialNextFocusGroups(conductor->parent_of_next_focus_group);
    conductor->is_choosing_next_group = true;

    conductor->selected_group_i_in_potential = conductor->potential_next_focus_groups.size() - 1;

    conductor->focus_group = conductor->potential_next_focus_groups[conductor->selected_group_i_in_potential];

    Group* subgroup = conductor->potential_next_focus_groups[conductor->selected_group_i_in_potential];
    bool subgroup_in_village = std::find(groups.begin(), groups.end(), subgroup) != groups.end();
    if (!subgroup_in_village) {
      for (auto voice : subgroup->voices) {
        voice->immediate_group = subgroup;
      }
      groups.push_back(subgroup);
    }
  }

  static inline void exitSubgroupMode(Conductor *conductor, std::vector<Group*> &groups) {
    Group* subgroup = conductor->potential_next_focus_groups[conductor->selected_group_i_in_potential];

    if (!subgroup->voices.empty()) {
      bool subgroup_in_village = std::find(groups.begin(), groups.end(), subgroup) != groups.end();
      if (!subgroup_in_village) {
        groups.push_back(subgroup);

        // TODO is it right?
        subgroup->voices[0]->immediate_group = subgroup;
      }
      conductor->focus_group = subgroup;
    } else {
      conductor->focus_group = conductor->parent_of_next_focus_group;
    }

    for (Group* group: conductor->potential_next_focus_groups) {
      bool subgroup_in_village = std::find(groups.begin(), groups.end(), group) != groups.end();
      if (!subgroup_in_village) {
        // for (auto voice: village->voices) {
        //   if (voice->immediate_group == group) {
        //     voice->immediate_group = conductor->parent_of_next_focus_group;
        //   }
        // }

        delete group;
      }
    }

    conductor->is_choosing_next_group = false;
  }

  static inline void selectNextPotentialFocusGroup(Conductor *conductor, std::vector<Group*> &groups) {
    assert(conductor->is_choosing_next_group);

    Group* last_subgroup = conductor->potential_next_focus_groups[conductor->selected_group_i_in_potential];
    bool subgroup_created = last_subgroup->voices.size() == 1;
    if (subgroup_created) {
      bool subgroup_in_village = std::find(groups.begin(), groups.end(), last_subgroup) != groups.end();
      if (subgroup_in_village) {
        last_subgroup->voices[0]->immediate_group = conductor->parent_of_next_focus_group;
      }
      groups.pop_back();
    }

    if (conductor->selected_group_i_in_potential == 0) {
      conductor->selected_group_i_in_potential = conductor->potential_next_focus_groups.size()-1;
    } else {
      conductor->selected_group_i_in_potential--;
    }

    Group* subgroup = conductor->potential_next_focus_groups[conductor->selected_group_i_in_potential];

    bool subgroup_in_village = std::find(groups.begin(), groups.end(), subgroup) != groups.end();
    if (!subgroup_in_village) {
      for (auto voice : subgroup->voices) {
        voice->immediate_group = subgroup;
      }
      groups.push_back(subgroup);
    }

    conductor->focus_group = subgroup;
  }

  static inline void ascend(Village *village) {
    if (village->conductor.is_choosing_next_group) {
      exitSubgroupMode(&village->conductor, village->groups);
    }

    if (village->conductor.focus_group->parent_group) {
      village->conductor.focus_group = village->conductor.focus_group->parent_group;
    }

    village->clearEmptyGroups();
  }
};

} // namespace hearth
} // namespace dsp
} // namespace kokopellivcv
