#pragma once

#include "Cycle.hpp"
#include "definitions.hpp"

#include <vector>

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct Group {
  Group *group;
  float love = 1.f;
  float relative_love = 1.f;

  char letter = 'A';

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
    Time adjusted_period = cycle->period;
    // TODO option
    float snap_back_window = this->beat_period / 2.f;
    // float snap_back_window = this->beat_period / 4.f;

    // TODO make option
    Time diff = cycle->period - period;
    if (0.f < diff) {
      // snap_back_window = this->beat_period;
      while (snap_back_window <= diff) {
        // TODO allow odds option?
        this->period *= 2;
        diff = cycle->period - this->period;
      }

      adjusted_period = this->period;
    } else {
      Time div = cycle->period / beat_period;
      int snap_beat = (int) div;
      float beat_phase = rack::math::eucMod(div, 1.0f);
      if (snap_back_window < beat_phase || snap_beat == 0) {
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

    printf("-- added to group %c, n_beats->%d\n", letter, convertToBeat(cycle->period, false));
  }

  inline void updateNextCyclesRelativeLove() {
    for (Cycle* cycle : this->cycles_in_group) {
      cycle->relative_love = 1.f;
    }

    for (unsigned int i = 0; i < this->cycles_in_group.size(); i++) {
      float cycle_i_relative_love = 1.f;
      for (unsigned int j = i + 1; j < this->cycles_in_group.size(); j++) {
        cycle_i_relative_love -= this->cycles_in_group[j]->readLove();
        if (cycle_i_relative_love <= 0.f)  {
          cycle_i_relative_love = 0.f;
          break;
        }
      }

      this->next_cycles_relative_love[i] = cycle_i_relative_love;
    }
  }

  static inline float smoothValue(float current, float old, float lambda) {
    return old + (current - old) * lambda;
  }

  inline void smoothStepCyclesRelativeLove(float lambda) {
    for (unsigned int i = 0; i < cycles_in_group.size(); i++) {
      cycles_in_group[i]->relative_love = smoothValue(next_cycles_relative_love[i], cycles_in_group[i]->relative_love, lambda);
    }
  }

  inline float read() {
    if (this->love == 0.f || this->relative_love == 0.f) {
      return 0.f;
    }

    kokopellivcv::dsp::SignalType signal_type = kokopellivcv::dsp::SignalType::AUDIO;
    float signal_out = 0.f;
    for (unsigned int i = 0; i < cycles_in_group.size(); i++) {
      float cycle_out = cycles_in_group[i]->readSignal();
      signal_out = kokopellivcv::dsp::sum(signal_out, cycle_out, signal_type);
    }

    return signal_out * love * relative_love;
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
