#pragma once

#include <vector>
#include <numeric> // std::iota

#include "rack.hpp"

#include "Cycle.hpp"
#include "Group.hpp"
#include "definitions.hpp"

namespace kokopellivcv {
namespace dsp {
namespace circle {

class Observer {
public:
  bool subgroup_addition = false;

  /* read only */
  bool _subgroup_mode = false;
  Group* _pivot_parent = nullptr;
  std::vector<Group*> _subgroups;
  int _focused_subgroup_i = -1;

  static inline std::vector<Group*> breakIntoSubgroups(Group* parent) {
    std::vector<Group*> subgroups;

    for (unsigned int i = 0; i < parent->cycles_in_group.size(); i++) {
      Cycle* cycle = parent->cycles_in_group[i];
      bool create_subgroup = cycle->immediate_group == parent;
      if (create_subgroup) {
        Group* subgroup = new Group();
        subgroups.push_back(subgroup);
        subgroup->parent_group = parent;
        subgroup->id = rack::string::f("%s-%d", parent->id.c_str(), i+1);
        subgroup->addExistingCycle(cycle);
        // cycle->immediate_group = subgroup;
      } else {
        if (cycle->immediate_group->parent_group == parent) {
          bool already_accounted_for = std::find(subgroups.begin(), subgroups.end(), cycle->immediate_group) != subgroups.end();
          if (!already_accounted_for) {
            subgroups.push_back(cycle->immediate_group);
          }
        }
      }
    }

    return subgroups;
  }

  static inline bool checkIfCycleInGroupOneIsObservedByCycleInGroupTwo(Group *one, Group *two) {
    if (!one) {
      return false;
    }

    if (one == two) {
      return true;
    }

    return checkIfCycleInGroupOneIsObservedByCycleInGroupTwo(one->parent_group, two);
  }

  inline bool checkIfInSubgroupMode() {
    return _subgroup_mode;
  }

  inline void tryEnterSubgroupMode(Song &song) {
    assert(!_subgroup_mode);
    assert(song.established);

    _pivot_parent = song.established;
    if (_pivot_parent->cycles_in_group.empty()) {
      return;
    }

    _subgroups = breakIntoSubgroups(_pivot_parent);
    _subgroup_mode = true;
    _focused_subgroup_i = _subgroups.size() - 1;
    song.established = _subgroups[_focused_subgroup_i];

    Group* subgroup = _subgroups[_focused_subgroup_i];
    bool subgroup_in_song = std::find(song.groups.begin(), song.groups.end(), subgroup) != song.groups.end();
    if (!subgroup_in_song) {
      for (auto cycle : subgroup->cycles_in_group) {
        cycle->immediate_group = subgroup;
      }
      song.groups.push_back(subgroup);
    }

  this->subgroup_addition = false;
}

inline void exitSubgroupMode(Song &song) {
  Group* subgroup = _subgroups[_focused_subgroup_i];
  if (!subgroup->cycles_in_group.empty()) {
    bool subgroup_in_song = std::find(song.groups.begin(), song.groups.end(), subgroup) != song.groups.end();
    if (!subgroup_in_song) {
      song.groups.push_back(subgroup);

      // TODO is it right?
      subgroup->cycles_in_group[0]->immediate_group = subgroup;
    }
    song.established = subgroup;
  } else {
    song.established = _pivot_parent;
  }

  for (Group* group: _subgroups) {
    bool subgroup_in_song = std::find(song.groups.begin(), song.groups.end(), group) != song.groups.end();
    if (!subgroup_in_song) {
      // for (auto cycle: song.cycles) {
      //   if (cycle->immediate_group == group) {
      //     cycle->immediate_group = _pivot_parent;
      //   }
      // }

      delete group;
    }
  }

  _subgroup_mode = false;
}

  inline void cycleSubgroup(Song &song) {
    assert(_subgroup_mode);

    Group* last_subgroup = _subgroups[_focused_subgroup_i];
    bool subgroup_created = last_subgroup->cycles_in_group.size() == 1;
    if (subgroup_created) {
      bool subgroup_in_song = std::find(song.groups.begin(), song.groups.end(), last_subgroup) != song.groups.end();
      if (subgroup_in_song) {
        last_subgroup->cycles_in_group[0]->immediate_group = _pivot_parent;
      }
      song.groups.pop_back();
    }

    if (_focused_subgroup_i == 0) {
      _focused_subgroup_i = _subgroups.size()-1;
    } else {
      _focused_subgroup_i--;
    }

    Group* subgroup = _subgroups[_focused_subgroup_i];

    bool subgroup_in_song = std::find(song.groups.begin(), song.groups.end(), subgroup) != song.groups.end();
    if (!subgroup_in_song) {
      for (auto cycle : subgroup->cycles_in_group) {
        cycle->immediate_group = subgroup;
      }
      song.groups.push_back(subgroup);
    }

    song.established = subgroup;
  }

  inline void ascend(Song &song) {
    if (_subgroup_mode) {
      exitSubgroupMode(song);
    }

    if (song.established->parent_group) {
      song.established = song.established->parent_group;
    }
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
