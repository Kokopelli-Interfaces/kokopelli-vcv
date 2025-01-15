#pragma once

#include "Cycle.hpp"
#include "definitions.hpp"

#include <vector>

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct Group {
  Group *parent_group = nullptr;

  std::string id = "*";

  std::vector<Cycle*> cycles_in_group;
  std::vector<float> next_cycles_relative_love;

  Time period = 1.f;
  Time beat_period = 1.f;

  // read only

  std::vector<Time> _period_history;

  inline bool isEmpty() {
    return cycles_in_group.empty();
  }

  inline void undoLastCycleWithoutUndoingParent() {
    assert(cycles_in_group.size() == _period_history.size());

    int last_i = cycles_in_group.size() - 1;
    cycles_in_group.pop_back();
    period = _period_history[last_i];
    _period_history.pop_back();
    next_cycles_relative_love.pop_back();
  }

  inline void undoLastCycle() {
    undoLastCycleWithoutUndoingParent();

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

  inline float getBeatPhase(Time time) {
    if (period != 0.f && beat_period != 0.f) {
      Time time_in_beat = std::fmod((float)time, (float)beat_period);
      return rack::clamp(time_in_beat / beat_period , 0.f, 1.f);
    }

    return 0.f;
  }

  inline float getPhase(Time time) {
    if (period != 0.f && beat_period != 0.f) {
      Time time_in_observed_sun = std::fmod((float)time, (float)period);
      return rack::clamp(time_in_observed_sun / period , 0.f, 1.f);
    }

    return 0.f;
  }

  inline int convertToBeat(Time time, bool mod) {
    if (period != 0.f && beat_period != 0.f) {
      float time_in_observed_sun = time;
      if (mod && period < time) {
        time_in_observed_sun = std::fmod((float)time, (float)period);
      }

      return (int) (time_in_observed_sun / beat_period);
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

  inline std::vector<int> getSnapBeats() {
    assert(beat_period != 0.f);

    std::vector<int> snap_beats;
    int beat = 0;
    int total_beats = period / beat_period;
    while(beat < total_beats) {
      beat++;
      if (total_beats % beat == 0) {
        snap_beats.push_back(beat);
      }
    }

    return snap_beats;
  }

  inline Time getAdjustedPeriod(Time cycle_period) {
    assert(!period == 0 && !beat_period == 0);

    Time adjusted_period;

    Time period_diff = cycle_period - period;
    if (0.f < period_diff) {
      Time start_period = period;
      Time new_period = start_period;
      Time percent_over = cycle_period / period;

      float period_round_back_percent = 1.5f;
      while (period_round_back_percent <= percent_over) {
        new_period += start_period;
        percent_over = percent_over - 1.f;
      }

      adjusted_period = new_period;
    } else {
      Time cycle_time_in_beats = cycle_period / beat_period;
      std::vector<int> snap_beats = getSnapBeats();
      int snap_beat = 1;
      for (unsigned int i = 1; i < snap_beats.size(); i++) {
        bool cycle_is_between_beats = (float)snap_beats[i-1] <= cycle_time_in_beats && cycle_time_in_beats <= (float)snap_beats[i];
        if (cycle_is_between_beats) {
          Time time_between_snap_beats = beat_period * (snap_beats[i] - snap_beats[i-1]);
          Time cycle_position_in_between_snap_beats_area = cycle_period - (beat_period * snap_beats[i-1]);
          Time percent_position_in_snap_beats_area = cycle_position_in_between_snap_beats_area / time_between_snap_beats;

          if (percent_position_in_snap_beats_area <= 0.5f) {
            snap_beat = snap_beats[i-1];
          } else {
            snap_beat = snap_beats[i];
          }
          break;
        }
      }

      Time snap_beat_time = (long double)snap_beat * beat_period;
      adjusted_period = snap_beat_time;
    }

    return adjusted_period;
  }

  inline void adjustPeriodsToFit(Time &cycle_period) {
    assert(!cycles_in_group.empty());

    Time adjusted_period = getAdjustedPeriod(cycle_period);
    if (period < adjusted_period) {
      period = adjusted_period;
    }

    cycle_period = adjusted_period;
  }

  // TODO cycle invariatn
  inline void addNewCycle(Cycle* cycle, bool use_ext_phase) {
    Time original_playhead = cycle->playhead;
    // Time original_period = cycle->_period;
    if (parent_group != nullptr && cycle->immediate_group != parent_group) {
      parent_group->addNewCycle(cycle, use_ext_phase);
    }

    this->cycles_in_group.push_back(cycle);
    this->next_cycles_relative_love.push_back(1.f);
    _period_history.push_back(period);

    if (cycle->loop) {
      if (this->cycles_in_group.size() == 1) {
        if (use_ext_phase) {
          period = 1.0f;
          beat_period = 1.0f;
        } else {
          period = cycle->_period;
          beat_period = cycle->_period;
          return;
        }
      }

      Time period_before = cycle->_period;
      adjustPeriodsToFit(cycle->_period);
      // printf("-- cycle period from (%Lf -> %Lf) (original %Lf)\n", period_before, cycle->_period, original_period);
      Time period_diff = period_before - cycle->_period;
      bool period_roundback = 0.f < period_diff;
      if (period_roundback) {
        cycle->playhead = period_diff;

        const Time fade_duration = 0.01;
        cycle->fader.triggerFadeIn(cycle->playhead, fade_duration);
        // printf("-- move playhead to %Lf (0 < (original)%Lf - (new)%Lf)\n", period_diff, period_before, cycle->_period);
      } else {
        cycle->playhead = original_playhead;
        // printf("-- move playhead to original spot %Lf ((original)%Lf - (new)%Lf < 0)\n", original_playhead, period_before, cycle->_period);
      }
    }

    // printf("-- added to group %s, n_beats->%d\n", id.c_str(), convertToBeat(cycle->_period, false));
    // printf("-- cycle period %Lf, cycle playhead %Lf\n", cycle->_period, cycle->playhead);
  }

  inline void addExistingCycle(Cycle* cycle) {
    this->cycles_in_group.push_back(cycle);
    this->next_cycles_relative_love.push_back(1.f);
    _period_history.push_back(period);

    Time period_before = cycle->_period;
    if (cycle->loop) {
      if (this->cycles_in_group.size() == 1) {
        period = cycle->_period;
        beat_period = cycle->_period;
      } else {
        adjustPeriodsToFit(cycle->_period);
      }
    }
    assert(cycle->_period == period_before);

    // printf("-- add existing cycle to group %s, n_beats->%d\n", id.c_str(), convertToBeat(cycle->_period, false));
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
