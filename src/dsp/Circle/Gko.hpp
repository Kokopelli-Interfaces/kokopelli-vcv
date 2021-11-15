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
  volatile float love_resolution = 10000.f;
  bool tune_to_frequency_of_established = true;

  /** read only */

  TimeAdvancer _song_time_advancer;
  TimeAdvancer _new_cycle_time_advancer;

  // PhaseOscillator _song_time_advancer;
  // PhaseAnalyzer _global_phase_analyzer;

  LoveDirection _love_direction = LoveDirection::ESTABLISHED;

  rack::dsp::ClockDivider _love_calculator_divider;
  std::vector<float> _next_cycles_love;

public:
  Gko() {
    _song_time_advancer.setTickFrequency(1.0f);
    _new_cycle_time_advancer.setTickFrequency(1.0f);
    _love_calculator_divider.setDivision(10000);
  }

private:
  static inline void fitCycleIntoGroup(Group* group, Cycle* cycle) {
  }

  inline void addCycle(std::vector<Cycle*> &cycles, Cycle* cycle) {
    cycles.push_back(cycle);
    _next_cycles_love.resize(cycles.size());
  }

public:
  inline void nextCycle(Song &song, CycleEnd cycle_end) {
    bool discard = false;
    switch (cycle_end) {
    case CycleEnd::DISCARD:
      discard = true;
      break;
    // case CycleEnd::DISCARD_AND_NEXT_MOVEMENT_IN_GROUP:
    //   song.current_movement = Movement::findNextMovementWithSameGroup(song.current_movement);
    //   song.playhead.tick = song.playhead.tick % song.current_movement->period;
    //   break;
    // case CycleEnd::DISCARD_AND_NEXT_MOVEMENT:
    //   song.current_movement = Movement::findNextMovement(song.current_movement);
    //   song.playhead.tick = song.playhead.tick % song.current_movement->period;
    //   break;
    case CycleEnd::JOIN_ESTABLISHED_LOOP:
      song.new_cycle->loop = true;
      // FIXME
      // fitCycleIntoGroup(song.current_movement, song.new_cycle);
      addCycle(song.cycles, song.new_cycle);
      break;
    case CycleEnd::JOIN_ESTABLISHED_NO_LOOP:
      // movement may have changed already, in which case, just leave it
      song.new_cycle->loop = false;
      addCycle(song.cycles, song.new_cycle);
      // if (song.new_cycle->movement->next == nullptr) {
      //   song.current_movement = Movement::createNextMovement(song.current_movement, song.current_movement->period);
      // }
      break;
    // case CycleEnd::SET_PERIOD_TO_ESTABLISHED_AND_EMERGE_WITH_MOVEMENT:
    //   song.new_cycle->loop = true;
    //   song.new_cycle->updateCyclePeriod(song.new_cycle->movement->end_tick - song.new_cycle->movement->start_tick);
    //   addCycle(song.cycles, song.new_cycle);
    //   break;
    // case CycleEnd::JOIN_ESTABLISHED_AND_CREATE_NEXT_MOVEMENT:
    //   song.new_cycle->loop = true;
    //   fitCycleIntoGroup(song.current_movement, song.new_cycle);
    //   addCycle(song.cycles, song.new_cycle);
    //   song.current_movement = Movement::createNextMovement(song.current_movement, song.current_movement->period);
      // song.playhead.tick = song.playhead.tick % song.current_movement->period;
      break;
    case CycleEnd::DO_NOT_LOOP_AND_NEXT_MOVEMENT:
      // movement may have changed already, in which case, just leave it
      song.new_cycle->loop = false;
      addCycle(song.cycles, song.new_cycle);
      // if (song.new_cycle->movement->next == nullptr) {
      //   song.current_movement = Movement::createNextMovement(song.current_movement, song.current_movement->period);
      // }
      break;
    }

    if (!discard) {
      song.new_cycle->finishWrite();
      // resizes
      song.established_group->addToGroup(song.new_cycle);
    } else {
      delete song.new_cycle;
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
    song.new_cycle = new Cycle(start, cycle_movement);
  }

  inline void undoCycle(Song &song) {
    if (0 < song.cycles.size()) {
      Cycle* last_cycle = song.cycles[song.cycles.size()-1];
      for (Group group : song.groups) {
        if (group.checkIfCycleIsInGroup(last_cycle)) {
          group.undoLastCycle();
        }
      }

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
  // TODO cycle types (song or movement), depends on tuning and affects love
  inline float smoothValue(float current, float old) {
    float lambda = this->love_resolution / 44100;
    return old + (current - old) * lambda;
  }


  inline void updateSongCyclesLove(std::vector<Cycle*> &cycles) {
    if (_love_calculator_divider.process()) {
      for (unsigned int cycle_i = 0; cycle_i < cycles.size(); cycle_i++) {

        float cycle_i_love = 1.f;
        for (unsigned int j = cycle_i + 1; j < cycles.size(); j++) {
          cycle_i_love -= cycles[j]->readLove();
          if (cycle_i_love <= 0.f)  {
            cycle_i_love = 0.f;
            break;
          }
        }

        _next_cycles_love[cycle_i] = cycle_i_love;
      }
    }

    for (unsigned int i = 0; i < cycles.size(); i++) {
      cycles[i]->relative_love = smoothValue(_next_cycles_love[i], cycles[i]->relative_love);
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
        cycle->playhead.restart();
      }
    }

    // FIXME
    // TimeEvent last_event = _song_time_advancer.getLastTimeEvent();
    // if (last_event == TimeEvent::NEXT_TICK) {
    //   // FIXME
    //   bool reached_movement_end = song.current_movement->period.tick <= song.playhead.tick;
    //   if (reached_movement_end) {
    //     bool next_movement = song.current_movement->next && !this->tune_to_frequency_of_established;
    //     if (next_movement) {
    //       song.current_movement = song.current_movement->next;
    //     } else {
    //       song.playhead = song.current_movement->start;
    //     }
    //   }

      _song_time_advancer.clearTimeEvent();
    // }
  }

public:
  inline void advance(Song &song, Inputs inputs) {
    advanceTime(song);

    LoveDirection new_love_direction = Inputs::getLoveDirection(inputs.love);
    if (_love_direction != new_love_direction) {
      handleLoveDirectionChange(song, new_love_direction);
    }

    song.new_cycle->write(inputs.in, inputs.love);

    updateSongCyclesLove(song.cycles);
  }

};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
