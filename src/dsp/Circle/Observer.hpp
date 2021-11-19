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
  /* read only */
  bool _subgroup_mode = false;
  Group* _pivot_parent = nullptr;
  int _focused_cycle_i_in_group = 0;

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

  inline Group* observeCycleInPivotParent(Cycle* cycle, std::vector<Group*> &groups) {
    Group* next_focus_group;
    if (cycle->immediate_group != _pivot_parent) {
      next_focus_group = cycle->immediate_group;
      printf("set to cycle immediate group %s\n", cycle->immediate_group->id.c_str());
    } else {
      printf("create new group with cycle\n");
      next_focus_group = new Group();
      groups.push_back(next_focus_group);
      next_focus_group->parent_group = _pivot_parent;
      next_focus_group->id = rack::string::f("%s.%d", _pivot_parent->id.c_str(), _focused_cycle_i_in_group+1);
      next_focus_group->addToGroupWithoutAddingToParents(cycle);
      cycle->immediate_group = next_focus_group;
    }

    return next_focus_group;
  }

  inline void enterSubgroupMode(Song &song) {
    assert(!_subgroup_mode);
    assert(song.established);

    _subgroup_mode = true;
    _pivot_parent = song.established;
    _focused_cycle_i_in_group = -1;

    if (song.established->cycles_in_group.empty()) {
      return;
    }

    _focused_cycle_i_in_group = _pivot_parent->cycles_in_group.size()-1;
    Cycle* focus_cycle = _pivot_parent->cycles_in_group[_focused_cycle_i_in_group];

    song.established = observeCycleInPivotParent(focus_cycle, song.groups);
  }

  inline void exitSubgroupModeByMakingSubgroupTheEstablishedGroup(Song &song) {
    song.established = song.new_cycle->immediate_group;
  }

  inline void exitSubgroupMode(Song &song) {
    assert(_subgroup_mode);
    _subgroup_mode = false;
    song.established = _pivot_parent;
  }

  inline void cycleObservedSubestablishment(Song &song) {
    assert(_subgroup_mode);

    int last_focused_cycle_i_in_group = _focused_cycle_i_in_group;
    Cycle* last_focus_cycle = _pivot_parent->cycles_in_group[last_focused_cycle_i_in_group];

    if (last_focus_cycle->immediate_group->cycles_in_group.size() == 1) {
      printf("Clearing last made group\n");
      last_focus_cycle->immediate_group->undoLastCycleWithoutUndoingParent();
      last_focus_cycle->immediate_group = _pivot_parent;
      song.clearEmptyGroups();
    }

    if (_focused_cycle_i_in_group == 0) {
      _focused_cycle_i_in_group = _pivot_parent->cycles_in_group.size()-1;
    } else {
      _focused_cycle_i_in_group--;
    }

    Cycle* focus_cycle = _pivot_parent->cycles_in_group[_focused_cycle_i_in_group];
    song.established = observeCycleInPivotParent(focus_cycle, song.groups);
  }

  inline void ascend(Song &song) {
    if (_subgroup_mode) {
      exitSubgroupMode(song);
    }

    if (song.established->parent_group) {
      song.established = song.established->parent_group;
    } else {
      Group* new_group = new Group();
      song.groups.push_back(new_group);
      new_group->parent_group = song.established->parent_group;
      new_group->letter = song.established->letter++;
      new_group->id = rack::string::f("%c", new_group->letter);
      song.established = new_group;
    }

    // nextCycle(song, CycleEnd::DISCARD);
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
