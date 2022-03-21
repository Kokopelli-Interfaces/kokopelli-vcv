#pragma once

#include <vector>
#include <numeric> // std::iota

#include "rack.hpp"

#include "Observer.hpp"
#include "Conductor.hpp"
#include "LoveUpdater.hpp"
#include "OutputUpdater.hpp"
#include "Village.hpp"
#include "Cycle.hpp"
#include "Group.hpp"
#include "definitions.hpp"
#include "Movement.hpp"
#include "TimeAdvancer.hpp"
#include "util/math.hpp"
#include "dsp/Signal.hpp"

namespace kokopellivcv {
namespace dsp {
namespace circle {

class Gko {
public:
  bool use_ext_phase = false;
  float ext_phase = 0.f;

  float sample_time = 1.0f;
  Time delay_shiftback = 0.f;
  bool tune_to_frequency_of_observed_sun = true;

  /** read only */

  Conductor conductor;
  Observer observer;
  LoveUpdater love_updater;
  OutputUpdater output_updater;

  bool _discard_cycle_at_next_love_return = false;

  float _last_ext_phase = 0.f;
  TimeAdvancer _time_advancer;

  LoveDirection _love_direction;

public:
  Gko() {
    _time_advancer.setTickFrequency(1.0f);
    // TODO set me when loop is observed_sun for consistent loops
  }

private:
  inline void addCycle(Village &village, Cycle* ended_cycle) {
    village.cycles.push_back(ended_cycle);
    ended_cycle->immediate_group->addNewCycle(ended_cycle);
    if (delay_shiftback < ended_cycle->period) {
      ended_cycle->playhead += delay_shiftback;
    }
  }

public:
  inline void nextCycle(Village &village, CycleEnd cycle_end) {
    Cycle* ended_cycle = village.new_cycle;

    // may happen when reverse recording
    ended_cycle->finishWrite();
    if (ended_cycle->period == 0.0) {
      delete ended_cycle;
      village.new_cycle = new Cycle(village.playhead, village.current_movement, village.observed_sun);
      return;
    }

    switch (cycle_end) {
    case CycleEnd::DISCARD:
      village.clearEmptyGroups();
      delete ended_cycle;
      break;
    case CycleEnd::NEXT_MOVEMENT_VIA_SHIFT:
      // ended_cycle->loop = false;
      // village.cycles.push_back(ended_cycle);
      // ended_cycle->immediate_group->addNewCycle(ended_cycle);
      conductor.nextMovement(village);
      delete ended_cycle;
      break;
    case CycleEnd::JOIN_OBSERVED_SUN_LOOP:
      ended_cycle->loop = true;
      addCycle(village, ended_cycle);
      break;
    case CycleEnd::SET_EQUAL_PERIOD_AND_JOIN_OBSERVED_SUN_LOOP:
      ended_cycle->loop = true;
      _discard_cycle_at_next_love_return = true;
      if (ended_cycle->immediate_group->period != 0.f) {
        ended_cycle->setPeriodToCaptureWindow(ended_cycle->immediate_group->period);
      }
      addCycle(village, ended_cycle);
      break;
    case CycleEnd::FLOOD:
      for (int i = village.cycles.size()-1; 0 <= i; i--) {
        if (Observer::checkIfCycleInGroupOneIsObservedByCycleInGroupTwo(village.cycles[i]->immediate_group, ended_cycle->immediate_group)) {
          village.cycles[i]->immediate_group->undoLastCycle();
          village.cycles.erase(village.cycles.begin() + i);
        }
      }

      // works better without

      _discard_cycle_at_next_love_return = true;

      delete ended_cycle;
      break;
    }

    // TODO
    Movement* cycle_movement;
    if (tune_to_frequency_of_observed_sun) {
      cycle_movement = village.current_movement;
    } else {
      // FIXME next movement
      cycle_movement = village.current_movement;
    }

    village.new_cycle = new Cycle(village.playhead, cycle_movement, village.observed_sun);
  }

  inline void undoCycle(Village &village) {
    if (observer.checkIfInSubgroupMode()) {
      observer.exitSubgroupMode(village);
    }

    if (0 < village.cycles.size()) {
      Cycle* most_recent_cycle = village.cycles[village.cycles.size()-1];
      most_recent_cycle->immediate_group->undoLastCycle();
      village.cycles.pop_back();
    }

    nextCycle(village, CycleEnd::DISCARD);
  }

  inline void cycleBackward(Village &village) {
    switch(_love_direction) {
    case LoveDirection::OBSERVED_SUN:
      if (observer.checkIfInSubgroupMode()) {
        if (observer.checkIfCanEnterFocusedSubgroup()) {
          observer.exitSubgroupMode(village);
        }
        nextCycle(village, CycleEnd::DISCARD);
      } else {
        nextCycle(village, CycleEnd::NEXT_MOVEMENT_VIA_SHIFT);
      }
      break;
    case LoveDirection::EMERGENCE:
      nextCycle(village, CycleEnd::DISCARD);
      break;
    case LoveDirection::NEW:
      nextCycle(village, CycleEnd::FLOOD);
      break;
    }
  }

  inline void cycleForward(Village &village) {
    switch(_love_direction) {
    case LoveDirection::OBSERVED_SUN:
      if (observer.checkIfInSubgroupMode()) {
        if (observer.checkIfCanEnterFocusedSubgroup()) {
          observer.exitSubgroupMode(village);
        }
        nextCycle(village, CycleEnd::DISCARD);
      } else {
        nextCycle(village, CycleEnd::NEXT_MOVEMENT_VIA_SHIFT);
      }
      break;
    case LoveDirection::EMERGENCE:
      nextCycle(village, CycleEnd::DISCARD);
      break;
    case LoveDirection::NEW:
      nextCycle(village, CycleEnd::FLOOD);
      break;
    }
  }

  inline void cycleObservation(Village &village) {
    switch(_love_direction) {
    case LoveDirection::OBSERVED_SUN:
      if (!observer.checkIfInSubgroupMode()) {
        observer.tryEnterSubgroupMode(village);
      } else {
        observer.cycleSubgroup(village);
      }
      nextCycle(village, CycleEnd::DISCARD);
      break;
    case LoveDirection::EMERGENCE:
    case LoveDirection::NEW:
      nextCycle(village, CycleEnd::SET_EQUAL_PERIOD_AND_JOIN_OBSERVED_SUN_LOOP);
      break;
    }
  }

  inline void handleLoveDirectionChange(Village &village, LoveDirection new_love_direction) {
    assert(new_love_direction != _love_direction);

    if (new_love_direction == LoveDirection::OBSERVED_SUN) {
      if (_discard_cycle_at_next_love_return) {
        nextCycle(village, CycleEnd::DISCARD);
        _discard_cycle_at_next_love_return = false;
      } else {
        nextCycle(village, CycleEnd::JOIN_OBSERVED_SUN_LOOP);
      }

      if (observer.checkIfInSubgroupMode()) {
        observer.exitSubgroupMode(village);
      }
    } else if (_love_direction == LoveDirection::OBSERVED_SUN && new_love_direction == LoveDirection::EMERGENCE) {
      nextCycle(village, CycleEnd::DISCARD);
    }

    _love_direction = new_love_direction;
  }

  inline void advanceTime(Village &village) {
    float step = this->sample_time;
    if (use_ext_phase) {
      step = ext_phase - _last_ext_phase;
      if (step < -0.95f) {
        step = ext_phase + 1 - _last_ext_phase;
      } else if (0.95f < step) {
        step = ext_phase - 1 - _last_ext_phase;
      }
      _last_ext_phase = ext_phase;
    }

    if (!village.cycles.empty()) {
      _time_advancer.step(village.playhead, step);
    } else {
      village.playhead = 0.f;
    }

    _time_advancer.step(village.new_cycle->playhead, step);
    if (use_ext_phase) {
      if (village.playhead < 0.f) {
        village.playhead = 0.f;
      }
      if (village.new_cycle->playhead < 0.f) {
        village.new_cycle->playhead = 0.f;
      }
    }

    for (Cycle* cycle : village.cycles) {
      _time_advancer.step(cycle->playhead, step);
      if (cycle->period < cycle->playhead) {
        while (cycle->period < cycle->playhead) {
        // printf("advanceTime: skip back cycle (%Lf < %Lf)\n", cycle->period, cycle->playhead);
          cycle->playhead -= cycle->period;
        }
      } else if (cycle->playhead < 0.f) {
        while (cycle->playhead < 0.f) {
        // printf("advanceTime: skip back cycle (%Lf < %Lf)\n", cycle->period, cycle->playhead);
          cycle->playhead += cycle->period;
        }
        cycle->playhead += cycle->period;
      }

      assert(0 <= cycle->playhead);
    }
  }

public:
  inline void advance(Village &village, Inputs inputs, Options options) {
    advanceTime(village);
    village.new_cycle->write(inputs.in, inputs.love);

    LoveDirection new_love_direction = Inputs::getLoveDirection(inputs.love);
    if (_love_direction != new_love_direction) {
      handleLoveDirectionChange(village, new_love_direction);
    }

    love_updater.updateVillageCyclesLove(village.cycles);
    output_updater.updateOutput(village.out, village.cycles, village.new_cycle->immediate_group, inputs, options);

    conductor.conduct(village);
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
