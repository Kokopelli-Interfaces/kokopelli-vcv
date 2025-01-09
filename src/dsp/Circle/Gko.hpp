#pragma once

#include <vector>
#include <numeric> // std::iota

#include "rack.hpp"

#include "Observer.hpp"
#include "LoveUpdater.hpp"
#include "OutputUpdater.hpp"
#include "Song.hpp"
#include "Cycle.hpp"
#include "Group.hpp"
#include "definitions.hpp"
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

  bool tune_to_frequency_of_observed_sun = true;

  /** read only */

  Observer observer;
  LoveUpdater love_updater;
  OutputUpdater output_updater;

  float _step_size = 0.0f;

  bool _discard_cycle_at_next_love_return = false;

  float _last_ext_phase = 0.f;

  LoveDirection _love_direction;

private:
  inline void addCycle(Song &song, Cycle* ended_cycle) {
    song.cycles.push_back(ended_cycle);
    ended_cycle->immediate_group->addNewCycle(ended_cycle, use_ext_phase);
    output_updater.antipop_filter.trigger();
  }

public:
  inline void nextCycle(Song &song, CycleEnd cycle_end) {
    Cycle* ended_cycle = song.new_cycle;

    // may happen when reverse recording
    ended_cycle->finishWrite();
    if (ended_cycle->period == 0.0) {
      delete ended_cycle;
      song.new_cycle = new Cycle(song.playhead, song.observed_sun);
      return;
    }

    switch (cycle_end) {
    case CycleEnd::DISCARD:
      if (observer.checkIfInSubgroupMode()) {
        if (ended_cycle->immediate_group->cycles_in_group.empty()) {
        observer.exitSubgroupMode(song);
        }
      }
      song.clearEmptyGroups();

      delete ended_cycle;
      break;
    case CycleEnd::JOIN_OBSERVED_SUN_LOOP:
      ended_cycle->loop = true;
      addCycle(song, ended_cycle);
      break;
    case CycleEnd::SET_EQUAL_PERIOD_AND_JOIN_OBSERVED_SUN_LOOP:
      ended_cycle->loop = true;
      _discard_cycle_at_next_love_return = true;
      if (ended_cycle->immediate_group->period != 0.f) {
        ended_cycle->setPeriodToCaptureWindow(ended_cycle->immediate_group->period);
      }
      addCycle(song, ended_cycle);
      break;
    case CycleEnd::FLOOD:
      for (int i = song.cycles.size()-1; 0 <= i; i--) {
        if (Observer::checkIfCycleInGroupOneIsObservedByCycleInGroupTwo(song.cycles[i]->immediate_group, ended_cycle->immediate_group)) {
          song.cycles[i]->immediate_group->undoLastCycle();
          song.cycles.erase(song.cycles.begin() + i);
        }
      }

      _discard_cycle_at_next_love_return = true;

      delete ended_cycle;
      break;
    }

    song.new_cycle = new Cycle(song.playhead, song.observed_sun);
  }

  inline void undoCycle(Song &song) {
    if (observer.checkIfInSubgroupMode()) {
      observer.exitSubgroupMode(song);
    }

    if (0 < song.cycles.size()) {
      Cycle* most_recent_cycle = song.cycles[song.cycles.size()-1];
      most_recent_cycle->immediate_group->undoLastCycle();
      song.cycles.pop_back();
      output_updater.antipop_filter.trigger();
    }

    nextCycle(song, CycleEnd::DISCARD);
  }

  inline void cycleForward(Song &song, Options options) {
    switch(_love_direction) {
    case LoveDirection::OBSERVED_SUN:
      if (observer.checkIfInSubgroupMode()) {
        if (observer.checkIfCanEnterFocusedSubgroup()) {
          observer.exitSubgroupMode(song);
        }
      }
      nextCycle(song, CycleEnd::DISCARD);
      break;
    case LoveDirection::EMERGENCE:
      nextCycle(song, CycleEnd::DISCARD);
      if (options.discard_cycle_on_change_return_after_refresh) {
        _discard_cycle_at_next_love_return = true;
      }
      break;
    case LoveDirection::NEW:
      nextCycle(song, CycleEnd::FLOOD);
      break;
    }
  }

  inline void cycleObservation(Song &song, bool forward) {
    switch(_love_direction) {
    case LoveDirection::OBSERVED_SUN:
      if (!observer.checkIfInSubgroupMode()) {
        observer.tryEnterSubgroupMode(song);
      } else {
        observer.cycleSubgroup(song, forward);
      }
      nextCycle(song, CycleEnd::DISCARD);
      break;
    case LoveDirection::EMERGENCE:
    case LoveDirection::NEW:
      nextCycle(song, CycleEnd::SET_EQUAL_PERIOD_AND_JOIN_OBSERVED_SUN_LOOP);
      break;
    }
  }

  inline void handleLoveDirectionChange(Song &song, LoveDirection new_love_direction) {
    assert(new_love_direction != _love_direction);

    if (new_love_direction == LoveDirection::OBSERVED_SUN) {
      if (_discard_cycle_at_next_love_return) {
        nextCycle(song, CycleEnd::DISCARD);
        _discard_cycle_at_next_love_return = false;
      } else {
        nextCycle(song, CycleEnd::JOIN_OBSERVED_SUN_LOOP);
      }

      if (observer.checkIfInSubgroupMode()) {
        observer.exitSubgroupMode(song);
      }
    } else if (_love_direction == LoveDirection::OBSERVED_SUN && new_love_direction == LoveDirection::EMERGENCE) {
      nextCycle(song, CycleEnd::DISCARD);
    }

    _love_direction = new_love_direction;
  }

  inline void advanceTime(Song &song) {
    float step = this->sample_time;
    if (_step_size > 0.0f) {
      step = _step_size;
    }

    if (use_ext_phase) {
      step = ext_phase - _last_ext_phase;
      if (step < -0.99f) {
        step = ext_phase + 1.0f - _last_ext_phase;
        output_updater.antipop_filter.trigger();
      } else if (0.99f < step) {
        step = ext_phase - 1.0f - _last_ext_phase;
        output_updater.antipop_filter.trigger();
      }
      _last_ext_phase = ext_phase;
      if (step != 0.f) {
        _step_size = step;
      }
    }

    if (!song.cycles.empty()) {
      song.playhead += step;
    } else {
      if (use_ext_phase) {
        song.playhead = ext_phase;
      } else {
        song.playhead = 0.f;
      }
    }

    if (_love_direction != LoveDirection::OBSERVED_SUN) {
      song.new_cycle->playhead += step;
    }

    if (use_ext_phase) {
      if (song.playhead < 0.f) {
        song.playhead = 0.f;
      }
      if (song.new_cycle->playhead < 0.f) {
        song.new_cycle->playhead = 0.f;
      }
    }

    for (Cycle* cycle : song.cycles) {
      cycle->playhead += step;
      if (cycle->period < cycle->playhead) {
        // // printf("advanceTime: skip back cycle (%Lf < %Lf)\n", cycle->period, cycle->playhead);
        cycle->playhead -= cycle->period;
        assert(cycle->playhead < cycle->period);
      } else if (cycle->playhead < 0.f) {
        cycle->playhead += cycle->period;
      }
    }
  }

public:
  inline void advance(Song &song, Inputs inputs, Options options) {
    float signal_in = inputs.in;

    advanceTime(song);
    song.new_cycle->write(signal_in, inputs.love);

    LoveDirection new_love_direction = Inputs::getLoveDirection(inputs.love);
    if (_love_direction != new_love_direction) {
      handleLoveDirectionChange(song, new_love_direction);
    }

    love_updater.updateSongCyclesLove(song.cycles);
    output_updater.updateOutput(song.out, song.cycles, song.new_cycle->immediate_group, signal_in, inputs.love, options);
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
