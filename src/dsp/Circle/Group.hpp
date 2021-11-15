#pragma once

#include "Cycle.hpp"
#include "definitions.hpp"

#include <vector>

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct Group {
  Group *group;

  char letter = 'A';

  std::vector<Cycle*> cycles_in_group;
  std::vector<Time> period_history;

  Time group_period;
  Time group_beat;

  inline void undoLastCycle() {
    assert(cycles_in_group.size() == period_history.size());

    int last_i = cycles_in_group.size() - 1;
    cycles_in_group.pop_back();
    group_period = period_history[last_i];
    period_history.pop_back();
  }

  inline bool checkIfCycleIsInGroup(Cycle* cycle) {
    for (Cycle* cycle_in_group : cycles_in_group) {
      if (cycle_in_group == cycle) {
        return true;
      }
    }
    return false;
  }

  inline Time fitPeriodIntoGroup(Time period) {
    Time diff = period - group_period;

    bool grow_group_and_period = 1 <= diff.tick;
    if (grow_group_and_period) {
      while (1 <= diff.tick) {
        // TODO optional
        this->group_period = this->group_period + this->group_period;
        diff = period - this->group_period;
      }
    }

    // TODO check divisions of group
    return this->group_period;
  }

  inline void addToGroup(Cycle* cycle) {
    this->cycles_in_group.push_back(cycle);
    this->period_history.push_back(group_period);

    if (cycle->loop) {
      if (this->cycles_in_group.size() == 1) {
        group_period = cycle->period;
        group_beat = cycle->period;
      } else {
        Time new_cycle_period = fitPeriodIntoGroup(cycle->period);
        cycle->updatePeriod(new_cycle_period);
      }
    }
  }

};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
