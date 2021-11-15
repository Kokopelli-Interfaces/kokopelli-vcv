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

  Time period = 0.f;
  Time beat_period = 0.f;

  inline void undoLastCycle() {
    assert(cycles_in_group.size() == period_history.size());

    int last_i = cycles_in_group.size() - 1;
    cycles_in_group.pop_back();
    period = period_history[last_i];
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

  inline int convertToBeat(Time time, bool mod) {
    if (period != 0.f && beat_period != 0.f) {
      float time_in_established = time;
      if (mod && period < time) {
        time_in_established = std::fmod((float)time, (float)period);
      }

      return (int) (time_in_established / beat_period);
    }
    return 0.f;
  }

  inline int getTotalBeats() {
    if (beat_period != 0.f) {
      return (period / beat_period);
    } else {
      return 0;
    }
  }

  inline void adjustPeriodsToFit(Cycle* cycle) {
    Time diff = cycle->period - period;

    float snap_back_window = 0.5f;
    bool grow_group_and_period = snap_back_window <= diff;
    if (grow_group_and_period) {
      while (snap_back_window <= diff) {
        this->period *= 2;
        diff = cycle->period - this->period;
      }
    }

    cycle->period = this->period;
  }

  inline void addToGroup(Cycle* cycle) {
    this->cycles_in_group.push_back(cycle);
    this->period_history.push_back(period);

    if (cycle->loop) {
      if (this->cycles_in_group.size() == 1) {
        period = cycle->period;
        beat_period = cycle->period;
      } else {
        adjustPeriodsToFit(cycle);
      }
    }

    printf("-- added to group %c, n_beats->%d\n", letter, convertToBeat(cycle->period, false));
  }

};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
