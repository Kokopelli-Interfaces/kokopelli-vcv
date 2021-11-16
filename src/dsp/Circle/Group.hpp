#pragma once

#include "Cycle.hpp"
#include "definitions.hpp"

#include <vector>

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct Group {
  Group *parent_group = nullptr;

  std::string id = "A";

  std::vector<Cycle*> cycles_in_group;
  std::vector<float> next_cycles_relative_love;

  std::vector<Time> period_history;

  Time period = 0.f;
  Time beat_period = 0.f;

  inline void undoLastCycle() {
    assert(cycles_in_group.size() == period_history.size());

    int last_i = cycles_in_group.size() - 1;
    cycles_in_group.pop_back();
    period = period_history[last_i];
    period_history.pop_back();
    next_cycles_relative_love.pop_back();

    if (parent_group) {
      parent_group->undoLastCycle();
    }
  }

  inline bool checkIfCycleIsInGroup(Cycle* cycle) {
    for (Cycle* cycle_in_group : cycles_in_group) {
      if (cycle_in_group == cycle) {
        return true;
      }
    }
    return false;
  }

  inline float getPhase(Time time) {
    if (period != 0.f && beat_period != 0.f) {
      Time time_in_established = std::fmod((float)time, (float)period);
      return rack::clamp(time_in_established / period , 0.f, 1.f);
    }
    return 0.f;
  }

  inline int convertToBeat(Time time, bool mod) {
    if (period != 0.f && beat_period != 0.f) {
      float time_in_established = time;
      if (mod && period < time) {
        time_in_established = std::fmod((float)time, (float)period);
      }

      return (int) (time_in_established / beat_period);
    }
    return 0;
  }

  inline int getTotalBeats() {
    if (beat_period != 0.f) {
      return (period / beat_period);
    } else {
      return 0;
    }
  }

  inline void adjustPeriodsToFit(Cycle* cycle) {
    Time adjusted_period;

    // TODO option

    Time diff = cycle->period - period;
    if (0.f < diff) {
      // snap_back_window = this->beat_period;
      Time percent_cycle = cycle->period / period;
      float period_round_back_phase = 1.5f;
      while (period_round_back_phase <= percent_cycle) {
        // TODO find lcms later
        this->period *= 2;
        diff = cycle->period - period;
        percent_cycle = cycle->period / period;
      }

      adjusted_period = this->period;
    } else {
      Time div = cycle->period / beat_period;
      float beat_phase = rack::math::eucMod(div, 1.0f);
      float beat_round_back_phase = 0.5f;
      int snap_beat = (int) div;
      if (beat_round_back_phase < beat_phase || snap_beat == 0) {
        snap_beat++;
      }

      int total_beats = getTotalBeats();
      while (total_beats % snap_beat != 0) {
        snap_beat++;
      }

      Time snap_beat_time = (long double)snap_beat * beat_period;
      diff = cycle->period - snap_beat_time;
      adjusted_period = snap_beat_time;
    }

    // preserve offset
    if (0.f < diff) {
      cycle->playhead = diff;
    }
    cycle->period = adjusted_period;
  }

  inline void addToGroup(Cycle* cycle) {
    if (parent_group) {
      parent_group->addToGroup(cycle);
    }

    this->cycles_in_group.push_back(cycle);
    this->next_cycles_relative_love.push_back(1.f);
    this->period_history.push_back(period);

    if (cycle->loop) {
      if (this->cycles_in_group.size() == 1) {
        period = cycle->period;
        beat_period = cycle->period;
      } else {
        adjustPeriodsToFit(cycle);
      }
    }

    printf("-- added to group %s, n_beats->%d\n", id.c_str(), convertToBeat(cycle->period, false));
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
