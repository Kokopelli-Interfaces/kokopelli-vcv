#pragma once

#include <vector>
#include <numeric> // std::iota

#include "rack.hpp"

#include "Voice.hpp"
#include "Group.hpp"
#include "GroupChanger.hpp"
#include "definitions.hpp"

namespace kokopellivcv {
namespace dsp {
namespace hearth {

class Observer {
public:
  /* read only */
  bool _subgroup_mode = false;
  Group* _pivot_parent = nullptr;
  std::vector<Group*> _subgroups;
  int _focused_subgroup_i = -1;

  static inline std::vector<Group*> breakIntoSubgroups(Group* parent) {
    std::vector<Group*> subgroups;

    for (unsigned int i = 0; i < parent->voices.size(); i++) {
      Voice* voice = parent->voices[i];
      bool create_subgroup = voice->immediate_group == parent;
      if (create_subgroup) {
        Group* subgroup = new Group();
        subgroups.push_back(subgroup);
        subgroup->parent_group = parent;
        subgroup->name = rack::string::f("%s/%d", parent->name.c_str(), i + 1);
        subgroup->movements.push_back(0.f);
        subgroup->voice_i_to_movement_i.push_back(0);

        GroupChanger::addExistingVoice(subgroup, voice);
        // voice->immediate_group = subgroup;
      } else {
        if (voice->immediate_group->parent_group == parent) {
          bool already_accounted_for = std::find(subgroups.begin(), subgroups.end(), voice->immediate_group) != subgroups.end();
          if (!already_accounted_for) {
            subgroups.push_back(voice->immediate_group);
          }
        }
      }
    }

    return subgroups;
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

  inline bool checkIfCanEnterFocusedSubgroup() {
    assert(_subgroup_mode);
    Group* focused_subgroup = _subgroups[_focused_subgroup_i];
    bool can_enter =  1 < focused_subgroup->voices.size();
    return can_enter;
  }

  inline void tryEnterSubgroupMode(Village &village) {
    assert(!_subgroup_mode);
    assert(village.observed_group);

    _pivot_parent = village.observed_group;
    if (_pivot_parent->voices.empty()) {
      return;
    }

    _subgroups = breakIntoSubgroups(_pivot_parent);
    _subgroup_mode = true;
    _focused_subgroup_i = _subgroups.size() - 1;
    village.observed_group = _subgroups[_focused_subgroup_i];

    Group* subgroup = _subgroups[_focused_subgroup_i];
    bool subgroup_in_village = std::find(village.groups.begin(), village.groups.end(), subgroup) != village.groups.end();
    if (!subgroup_in_village) {
      for (auto voice : subgroup->voices) {
        voice->immediate_group = subgroup;
      }
      village.groups.push_back(subgroup);
    }
  }

inline void exitSubgroupMode(Village &village) {
  Group* subgroup = _subgroups[_focused_subgroup_i];

  if (!subgroup->voices.empty()) {
    bool subgroup_in_village = std::find(village.groups.begin(), village.groups.end(), subgroup) != village.groups.end();
    if (!subgroup_in_village) {
      village.groups.push_back(subgroup);

      // TODO is it right?
      subgroup->voices[0]->immediate_group = subgroup;
    }
    village.observed_group = subgroup;
  } else {
    village.observed_group = _pivot_parent;
  }

  for (Group* group: _subgroups) {
    bool subgroup_in_village = std::find(village.groups.begin(), village.groups.end(), group) != village.groups.end();
    if (!subgroup_in_village) {
      // for (auto voice: village.voices) {
      //   if (voice->immediate_group == group) {
      //     voice->immediate_group = _pivot_parent;
      //   }
      // }

      delete group;
    }
  }

  _subgroup_mode = false;
}

  inline void voicesubgroup(Village &village) {
    assert(_subgroup_mode);

    Group* last_subgroup = _subgroups[_focused_subgroup_i];
    bool subgroup_created = last_subgroup->voices.size() == 1;
    if (subgroup_created) {
      bool subgroup_in_village = std::find(village.groups.begin(), village.groups.end(), last_subgroup) != village.groups.end();
      if (subgroup_in_village) {
        last_subgroup->voices[0]->immediate_group = _pivot_parent;
      }
      village.groups.pop_back();
    }

    if (_focused_subgroup_i == 0) {
      _focused_subgroup_i = _subgroups.size()-1;
    } else {
      _focused_subgroup_i--;
    }

    Group* subgroup = _subgroups[_focused_subgroup_i];

    bool subgroup_in_village = std::find(village.groups.begin(), village.groups.end(), subgroup) != village.groups.end();
    if (!subgroup_in_village) {
      for (auto voice : subgroup->voices) {
        voice->immediate_group = subgroup;
      }
      village.groups.push_back(subgroup);
    }

    village.observed_group = subgroup;
  }

  inline void ascend(Village &village) {
    if (_subgroup_mode) {
      exitSubgroupMode(village);
    }

    if (village.observed_group->parent_group) {
      village.observed_group = village.observed_group->parent_group;
    }

    village.clearEmptyGroups();
  }
};

} // namespace hearth
} // namespace dsp
} // namespace kokopellivcv
