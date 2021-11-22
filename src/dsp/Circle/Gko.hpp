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
#include "Movement.hpp"
#include "TimeAdvancer.hpp"
#include "util/math.hpp"
#include "dsp/Signal.hpp"

namespace kokopellivcv {
namespace dsp {
namespace circle {

/**
   The advancer of the song. Gko - 'the one that emanates the universe'.
*/
class Gko {
public:
  bool use_ext_phase = false;
  float ext_phase = 0.f;

  float sample_time = 1.0f;
  float love_resolution = 1000.f;
  bool tune_to_frequency_of_established = true;

  /** read only */

  Observer observer;
  LoveUpdater love_updater;
  OutputUpdater output_updater;

  bool _equal_period_addition = false;

  float _last_ext_phase = 0.f;
  TimeAdvancer _time_advancer;

  LoveDirection _love_direction;

public:
  Gko() {
    _time_advancer.setTickFrequency(1.0f);
    // TODO set me when loop is established for consistent loops
  }

public:
  inline void nextCycle(Song &song, CycleEnd cycle_end) {
    Cycle* ended_cycle = song.new_cycle;
    ended_cycle->finishWrite();

    // may happen when reverse recording
    if (ended_cycle->period == 0.f) {
      delete ended_cycle;
      song.new_cycle = new Cycle(song.playhead, song.current_movement, song.established);
      return;
    }

    switch (cycle_end) {
    case CycleEnd::DISCARD:
      song.clearEmptyGroups();
      delete ended_cycle;
      break;
    case CycleEnd::JOIN_ESTABLISHED_LOOP:
      ended_cycle->loop = true;
      song.cycles.push_back(ended_cycle);
      ended_cycle->immediate_group->addNewCycle(ended_cycle);
      break;
    case CycleEnd::SET_EQUAL_PERIOD_AND_JOIN_ESTABLISHED_LOOP:
      ended_cycle->loop = true;
      _equal_period_addition = true;
      ended_cycle->finishWindowCaptureWrite(ended_cycle->immediate_group->period);
      song.cycles.push_back(ended_cycle);
      ended_cycle->immediate_group->addNewCycle(ended_cycle);
      break;
    case CycleEnd::JOIN_ESTABLISHED_NO_LOOP:
      // FIXME
      delete ended_cycle;
      // ended_cycle->loop = false;
      // song.cycles.push_back(ended_cycle);
      // ended_cycle->immediate_group->addNewCycle(ended_cycle);
      break;
    case CycleEnd::FLOOD:
      for (int i = song.cycles.size()-1; 0 <= i; i--) {
        if (Observer::checkIfCycleInGroupOneIsObservedByCycleInGroupTwo(song.cycles[i]->immediate_group, ended_cycle->immediate_group)) {
          song.cycles[i]->immediate_group->undoLastCycle();
          song.cycles.erase(song.cycles.begin() + i);
        }
      }

      if (observer.checkIfInSubgroupMode()) {
        observer.exitSubgroupMode(song);
      }
      song.clearEmptyGroups();

      delete ended_cycle;
      break;
    }

    // TODO
    Movement* cycle_movement;
    if (tune_to_frequency_of_established) {
      cycle_movement = song.current_movement;
    } else {
      // FIXME next movement
      cycle_movement = song.current_movement;
    }

    song.new_cycle = new Cycle(song.playhead, cycle_movement, song.established);
  }

  inline void undoCycle(Song &song) {
    if (observer.checkIfInSubgroupMode()) {
      observer.exitSubgroupMode(song);
    }

    if (0 < song.cycles.size()) {
      Cycle* most_recent_cycle = song.cycles[song.cycles.size()-1];
      most_recent_cycle->immediate_group->undoLastCycle();
      song.cycles.pop_back();
    }
  }

  inline void cycleForward(Song &song) {
    switch(_love_direction) {
    case LoveDirection::ESTABLISHED:
      if (observer.checkIfInSubgroupMode()) {
        Group* focused_subgroup = observer._subgroups[observer._focused_subgroup_i];
        bool move_into_subgroup =  1 < focused_subgroup->cycles_in_group.size();
        if (move_into_subgroup) {
          observer.exitSubgroupMode(song);
        }
        nextCycle(song, CycleEnd::DISCARD);
      } else {
        nextCycle(song, CycleEnd::JOIN_ESTABLISHED_NO_LOOP);
      }
      break;
    case LoveDirection::EMERGENCE:
      nextCycle(song, CycleEnd::JOIN_ESTABLISHED_NO_LOOP);
      break;
    case LoveDirection::NEW:
      nextCycle(song, CycleEnd::FLOOD);
      break;
    }
  }

  inline void cycleObservation(Song &song) {
    switch(_love_direction) {
    case LoveDirection::ESTABLISHED:
      if (!observer.checkIfInSubgroupMode()) {
        observer.tryEnterSubgroupMode(song);
      } else {
        observer.cycleSubgroup(song);
      }
      nextCycle(song, CycleEnd::DISCARD);
      break;
    case LoveDirection::EMERGENCE:
    case LoveDirection::NEW:
      nextCycle(song, CycleEnd::SET_EQUAL_PERIOD_AND_JOIN_ESTABLISHED_LOOP);
      break;
    }
  }

  inline void handleLoveDirectionChange(Song &song, LoveDirection new_love_direction) {
    assert(new_love_direction != _love_direction);

    if (new_love_direction == LoveDirection::ESTABLISHED) {
      if (_equal_period_addition) {
        nextCycle(song, CycleEnd::DISCARD);
      } else {
        nextCycle(song, CycleEnd::JOIN_ESTABLISHED_LOOP);
      }

      _equal_period_addition = false;
    } else if (_love_direction == LoveDirection::ESTABLISHED && new_love_direction == LoveDirection::EMERGENCE) {
      nextCycle(song, CycleEnd::DISCARD);
    }

    _love_direction = new_love_direction;
  }

  inline void advanceTime(Song &song) {
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

    _time_advancer.step(song.playhead, step);
    _time_advancer.step(song.new_cycle->playhead, step);
    if (use_ext_phase) {
      if (song.playhead < 0.f) {
        song.playhead = 0.f;
      }
      if (song.new_cycle->playhead < 0.f) {
        song.new_cycle->playhead = 0.f;
      }
    }

    for (Cycle* cycle : song.cycles) {
      _time_advancer.step(cycle->playhead, step);
      if (cycle->period < cycle->playhead) {
        cycle->playhead -= cycle->period;
      } else if (cycle->playhead < 0.f) {
        cycle->playhead += cycle->period;
      }
    }
  }

public:
  inline void advance(Song &song, Inputs inputs) {
    advanceTime(song);
    song.new_cycle->write(inputs.in, inputs.love);

    LoveDirection new_love_direction = Inputs::getLoveDirection(inputs.love);
    if (_love_direction != new_love_direction) {
      handleLoveDirectionChange(song, new_love_direction);
    }

    love_updater.updateSongCyclesLove(song.cycles);
    output_updater.updateOutput(song.out, song.cycles, song.new_cycle->immediate_group, inputs);
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
