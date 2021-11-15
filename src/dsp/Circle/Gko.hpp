#pragma once

#include <vector>
#include <numeric> // std::iota

#include "Song.hpp"
#include "Cycle.hpp"
#include "Group.hpp"
#include "definitions.hpp"
#include "TimeCapture.hpp"
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
  float love_resolution = 10000.f;
  bool tune_to_frequency_of_established = true;

  /** read only */

  TimeAdvancer _song_time_advancer;
  TimeAdvancer _new_cycle_time_advancer;

  // PhaseOscillator _song_time_advancer;
  // PhaseAnalyzer _global_phase_analyzer;

  LoveDirection _love_direction = LoveDirection::ESTABLISHED;

  rack::dsp::ClockDivider _love_calculator_divider;

public:
  Gko() {
    _song_time_advancer.setTickFrequency(1.0f);
    _new_cycle_time_advancer.setTickFrequency(1.0f);
    _love_calculator_divider.setDivision(10000);
  }

private:
  inline void addCycle(std::vector<Cycle*> &cycles, Cycle* cycle) {
    cycle->finishWrite();
    cycle->group->addToGroup(cycle);
    cycles.push_back(cycle);
  }

public:
  inline void nextCycle(Song &song, CycleEnd cycle_end) {
    switch (cycle_end) {
    case CycleEnd::DISCARD:
      delete song.new_cycle;
      break;
    case CycleEnd::JOIN_ESTABLISHED_LOOP:
      song.new_cycle->loop = true;
      addCycle(song.cycles, song.new_cycle);
      break;
    case CycleEnd::JOIN_ESTABLISHED_NO_LOOP:
      song.new_cycle->loop = false;
      addCycle(song.cycles, song.new_cycle);
      break;
    case CycleEnd::DO_NOT_LOOP_AND_NEXT_MOVEMENT:
      song.new_cycle->loop = false;
      addCycle(song.cycles, song.new_cycle);
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
    song.new_cycle = new Cycle(start, cycle_movement, song.established_group);
  }

  inline void undoCycle(Song &song) {
    if (0 < song.cycles.size()) {
      Cycle* last_cycle = song.cycles[song.cycles.size()-1];
      last_cycle->group->undoLastCycle();
      song.cycles.pop_back();
    }
  }

  inline void cycleForward(Song &song) {
    switch(_love_direction) {
    case LoveDirection::ESTABLISHED:
      nextCycle(song, CycleEnd::JOIN_ESTABLISHED_NO_LOOP);
      break;
    case LoveDirection::EMERGENCE:
      nextCycle(song, CycleEnd::JOIN_ESTABLISHED_NO_LOOP);
      break;
    case LoveDirection::NEW:
      nextCycle(song, CycleEnd::JOIN_ESTABLISHED_NO_LOOP);
      // nextCycle(song, CycleEnd::DO_NOT_LOOP_AND_NEXT_MOVEMENT);
      // TODO
      // nextCycle(song, CycleEnd::SET_TO_SONG_PERIOD_AND_NEXT_GROUP);
      break;
    }
  }

private:
  inline void updateSongCyclesLove(std::vector<Group> &groups) {
    if (_love_calculator_divider.process()) {
      for (Group group : groups) {
        group.updateNextCyclesRelativeLove();
      }
    }

    for (Group group : groups) {
      float lambda = love_resolution / 44100;
      group.smoothStepCyclesRelativeLove(lambda);
    }
  }

  inline void handleLoveDirectionChange(Song &song, LoveDirection new_love_direction) {
    assert(new_love_direction != _love_direction);

    // if (this->tune_to_frequency_of_established) {
    //   if (new_love_direction == LoveDirection::ESTABLISHED) {
    //     nextCycle(song, CycleEnd::JOIN_ESTABLISHED);
    //   } else if (_love_direction == LoveDirection::ESTABLISHED && new_love_direction == LoveDirection::EMERGENCE) {
    //     nextCycle(song, CycleEnd::DISCARD);
    //   }
    // }

    if (new_love_direction == LoveDirection::ESTABLISHED) {
      nextCycle(song, CycleEnd::JOIN_ESTABLISHED_LOOP);
    } else if (_love_direction == LoveDirection::ESTABLISHED && new_love_direction == LoveDirection::EMERGENCE) {
      nextCycle(song, CycleEnd::DISCARD);
      // nextCycle(song, CycleEnd::JOIN_ESTABLISHED_LOOP);
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

    // TODO cycle back
  }

public:
  inline void advance(Song &song, Inputs inputs) {
    advanceTime(song);

    LoveDirection new_love_direction = Inputs::getLoveDirection(inputs.love);
    if (_love_direction != new_love_direction) {
      handleLoveDirectionChange(song, new_love_direction);
    }

    song.new_cycle->write(inputs.in, inputs.love);

    updateSongCyclesLove(song.groups);
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
