#pragma once

#include <vector>
#include <numeric> // std::iota

#include "Song.hpp"
#include "Cycle.hpp"
#include "Group.hpp"
#include "definitions.hpp"
#include "Recording.hpp"
#include "Movement.hpp"
#include "TimeAdvancer.hpp"
#include "util/math.hpp"
#include "dsp/Signal.hpp"

namespace kokopellivcv {
namespace dsp {
namespace circle {

/**
   The advancer of the song
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
  std::vector<float> _next_cycles_love;

public:
  Gko() {
    _song_time_advancer.setTickFrequency(0.01f);
    _new_cycle_time_advancer.setTickFrequency(0.01f);
    _love_calculator_divider.setDivision(10000);
  }

private:
  static inline void fitCycleIntoGroup(Group* group, Cycle* cycle) {
    // unsigned int n_movement_ticks = period.ticks;
    // if (cycle->period.tick == n_movement_ticks) {
    //   return;
    // }

    // bool grow_movement = n_movement_ticks < cycle->period.tick;
    // if (grow_movement) {
    //   unsigned int new_n_movement_ticks = n_movement_ticks;
    //   while (new_n_movement_ticks < cycle->period.tick) {
    //     new_n_movement_ticks += n_movement_ticks;
    //   }
    //   cycle->period.tick = new_n_movement_ticks;
    //   movement->end_tick = movement->start_tick + new_n_movement_ticks;
    // } else {
    //   cycle->period.tick = kokopellivcv::util::findNumberAfterFirstThatFitsIntoSecond(cycle->period.tick, n_movement_ticks);
    // }
  }

  inline void addCycle(std::vector<Cycle*> &cycles, Cycle* cycle) {
    cycles.push_back(cycle);
    _next_cycles_love.resize(cycles.size());
  }

public:
  inline void nextCycle(Song &song, CycleEnd cycle_end) {
    switch (cycle_end) {
    case CycleEnd::DISCARD:
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
    // case CycleEnd::SET_TO_SONG_PERIOD_AND_NEXT_GROUP:
    //   // TODO
    //   song.new_cycle->loop = false;
    //   addCycle(song.cycles, song.new_cycle);
    //   if (song.new_cycle->movement->next == nullptr) {
    //     song.current_movement = Movement::createNextGroupMovement(song.current_movement, 1);
    //     song.playhead.tick = 0;
    //   }
    //   break;
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

  inline void undoCycle(std::vector<Cycle*> &cycles) {
    if (0 < cycles.size()) {
      cycles.erase(cycles.end()-1);
    }
  }

  inline void cycleForward(Song &song) {
    switch(_love_direction) {
    case LoveDirection::ESTABLISHED:
      // if (this->tune_to_frequency_of_established) {
      //   nextCycle(song, CycleEnd::DISCARD_AND_NEXT_MOVEMENT_IN_GROUP);
      // } else {
      //   nextCycle(song, CycleEnd::DISCARD_AND_NEXT_MOVEMENT);
      //   // A -> B
      //   // TODO should it place the recording ?
      //   // nextCycle(song, false);
      // }
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


  inline void updateSongCyclesLove(Song &song) {
    if (_love_calculator_divider.process()) {
      for (unsigned int cycle_i = 0; cycle_i < song.cycles.size(); cycle_i++) {

        float cycle_i_love = 1.f;
        for (unsigned int j = cycle_i + 1; j < song.cycles.size(); j++) {
          cycle_i_love -= song.cycles[j]->readLove(song.playhead);
          if (cycle_i_love <= 0.f)  {
            cycle_i_love = 0.f;
            break;
          }
        }

        _next_cycles_love[cycle_i] = cycle_i_love;
      }
    }

    for (unsigned int i = 0; i < song.cycles.size(); i++) {
      song.cycles[i]->love = smoothValue(_next_cycles_love[i], song.cycles[i]->love);
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

    updateSongCyclesLove(song);
  }

};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv