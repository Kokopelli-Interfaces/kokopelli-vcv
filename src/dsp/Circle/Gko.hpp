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

  TimeAdvancer _song_time_advancer;
  TimeAdvancer _new_cycle_time_advancer;

  LoveDirection _love_direction = LoveDirection::ESTABLISHED;

public:
  Gko() {
    _song_time_advancer.setTickFrequency(1.0f);
    _new_cycle_time_advancer.setTickFrequency(1.0f);
    // TODO set me when loop is established for consistent loops
  }

public:
  inline void nextCycle(Song &song, CycleEnd cycle_end) {
    Cycle* ended_cycle = song.new_cycle;

    switch (cycle_end) {
    case CycleEnd::DISCARD:
      song.clearEmptyGroups();
      delete ended_cycle;
      break;
    case CycleEnd::JOIN_ESTABLISHED_LOOP:
      ended_cycle->loop = true;
      ended_cycle->finishWrite();
      song.cycles.push_back(ended_cycle);
      ended_cycle->immediate_group->addToGroup(ended_cycle);
      break;
    case CycleEnd::SET_EQUAL_PERIOD_AND_JOIN_ESTABLISHED_LOOP:
      ended_cycle->loop = true;

      ended_cycle->finishWindowCaptureWrite(ended_cycle->immediate_group->period);
      song.cycles.push_back(ended_cycle);
      ended_cycle->immediate_group->addToGroup(ended_cycle);
      break;
    case CycleEnd::JOIN_ESTABLISHED_NO_LOOP:
      // FIXME
      delete ended_cycle;
      // ended_cycle->loop = false;
      // song.cycles.push_back(ended_cycle);
      // ended_cycle->immediate_group->addToGroup(ended_cycle);
      break;
    case CycleEnd::FLOOD:
      for (int i = song.cycles.size()-1; 0 <= i; i--) {
        if (Observer::checkIfCycleInGroupOneIsObservedByCycleInGroupTwo(song.cycles[i]->immediate_group, ended_cycle->immediate_group)) {
          song.cycles[i]->immediate_group->undoLastCycle();
          song.cycles.erase(song.cycles.begin() + i);
        }
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

    Time start = song.playhead;
    // FIXME calculated samples_per_tick will suck
    song.new_cycle = new Cycle(start, cycle_movement, song.established);
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
        observer.exitSubgroupModeByMakingSubgroupTheEstablishedGroup(song);
        nextCycle(song, CycleEnd::DISCARD);
      } else {
        nextCycle(song, CycleEnd::JOIN_ESTABLISHED_NO_LOOP);
      }
      break;
    case LoveDirection::EMERGENCE:
      if (observer.checkIfInSubgroupMode()) {
        nextCycle(song, CycleEnd::SET_EQUAL_PERIOD_AND_JOIN_ESTABLISHED_LOOP);
      } else {
        nextCycle(song, CycleEnd::JOIN_ESTABLISHED_NO_LOOP);
      }
      break;
    case LoveDirection::NEW:
      if (observer.checkIfInSubgroupMode()) {
        nextCycle(song, CycleEnd::SET_EQUAL_PERIOD_AND_JOIN_ESTABLISHED_LOOP);
      } else {
        nextCycle(song, CycleEnd::FLOOD);
      }
      // nextCycle(song, CycleEnd::DO_NOT_LOOP_AND_NEXT_MOVEMENT);
      // TODO
      // nextCycle(song, CycleEnd::SET_TO_SONG_PERIOD_AND_NEXT_GROUP);
      break;
    }
  }

  inline void cycleObservation(Song &song) {
    switch(_love_direction) {
    case LoveDirection::ESTABLISHED:
      if (!observer.checkIfInSubgroupMode()) {
        observer.enterSubgroupMode(song);
      } else {
        observer.cycleObservedSubestablishment(song);
      }
      nextCycle(song, CycleEnd::DISCARD);
      break;
    case LoveDirection::EMERGENCE:
      // TODO
      // if (!observer.checkIfInSubgroupMode()) {
      //   observer.enterSubgroupMode(song.new_cycle.immediate_group);
      // } else {
      //   observer.observePreviousCycleInEstablished(song);
      // }
      break;
    case LoveDirection::NEW:
      // TODO next group?
      // nextCycle(song, CycleEnd::JOIN_ESTABLISHED_NO_LOOP);
      break;
    }
  }

  inline void handleLoveDirectionChange(Song &song, LoveDirection new_love_direction) {
    assert(new_love_direction != _love_direction);

    if (new_love_direction == LoveDirection::ESTABLISHED) {
      if (observer.checkIfInSubgroupMode()) {
        observer.exitSubgroupMode(song);
        nextCycle(song, CycleEnd::DISCARD);
      } else {
        nextCycle(song, CycleEnd::JOIN_ESTABLISHED_LOOP);
      }
    } else if (_love_direction == LoveDirection::ESTABLISHED && new_love_direction == LoveDirection::EMERGENCE) {
      nextCycle(song, CycleEnd::DISCARD);
    }

    _love_direction = new_love_direction;
  }

  inline void advanceTime(Song &song) {
    _song_time_advancer.step(song.playhead, this->sample_time);
    _new_cycle_time_advancer.step(song.new_cycle->playhead, this->sample_time);

    for (Cycle* cycle : song.cycles) {
      _song_time_advancer.step(cycle->playhead, this->sample_time);
      if (cycle->period < cycle->playhead) {
        cycle->playhead = 0.f;
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
