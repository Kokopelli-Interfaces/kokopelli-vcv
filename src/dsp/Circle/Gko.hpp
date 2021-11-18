#pragma once

#include <vector>
#include <numeric> // std::iota

#include "rack.hpp"

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
  float love_resolution = 1000.f;
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
    // TODO set me when loop is established for consistent loops
    _love_calculator_divider.setDivision(5000);
  }

private:
  inline void addCycle(std::vector<Cycle*> &cycles, Cycle* cycle) {
    cycle->finishWrite();
    cycles.push_back(cycle);
    _next_cycles_love.push_back(0.f);
  }

public:
  inline void nextCycle(Song &song, CycleEnd cycle_end) {
    Cycle* ended_cycle = song.new_cycle;

    switch (cycle_end) {
    case CycleEnd::DISCARD:
      delete ended_cycle;
      break;
    case CycleEnd::JOIN_ESTABLISHED_LOOP:
      ended_cycle->loop = true;
      addCycle(song.cycles, ended_cycle);
      ended_cycle->group->addToGroup(ended_cycle);
      break;
    case CycleEnd::JOIN_ESTABLISHED_NO_LOOP:
      // FIXME
      delete ended_cycle;
      // ended_cycle->loop = false;
      // addCycle(song.cycles, ended_cycle);
      // ended_cycle->group->addToGroup(ended_cycle);
      break;
    case CycleEnd::FLOOD:
      for (int i = song.cycles.size()-1; 0 <= i; i--) {
        if (checkIfCycleInGroupOneIsObservedByCycleInGroupTwo(song.cycles[i]->group, ended_cycle->group)) {
          song.cycles[i]->group->undoLastCycle();
          song.cycles.erase(song.cycles.begin() + i);
        }
      }

      // FIXME add non-loop
      delete ended_cycle;
      // ended_cycle->loop = false;
      // addCycle(song.cycles, ended_cycle);
      // ended_cycle->group->addToGroup(ended_cycle);

      break;
    case CycleEnd::JOIN_ESTABLISHED_AND_CREATE_SUBGROUP:
      ended_cycle->loop = true;

      Group* parent_group = ended_cycle->group;
      Group* subgroup = new Group();
      song.groups.push_back(subgroup);
      subgroup->parent_group = parent_group;
      subgroup->id = rack::string::f("%s%d", parent_group->id.c_str(), parent_group->cycles_in_group.size());

      ended_cycle->group = subgroup;
      addCycle(song.cycles, ended_cycle);
      song.established_group = subgroup;
      subgroup->addToGroup(ended_cycle);
      break;
    // case CycleEnd::NEW:
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
      nextCycle(song, CycleEnd::FLOOD);
      // nextCycle(song, CycleEnd::DO_NOT_LOOP_AND_NEXT_MOVEMENT);
      // TODO
      // nextCycle(song, CycleEnd::SET_TO_SONG_PERIOD_AND_NEXT_GROUP);
      break;
    }
  }

  inline void cycleDivinity(Song &song) {
    switch(_love_direction) {
    case LoveDirection::ESTABLISHED:
      // TODO
      if (song.established_group->parent_group) {
        nextCycle(song, CycleEnd::DISCARD);
        song.established_group = song.established_group->parent_group;
      }
      break;
    case LoveDirection::EMERGENCE:
      nextCycle(song, CycleEnd::JOIN_ESTABLISHED_AND_CREATE_SUBGROUP);
      break;
    case LoveDirection::NEW:
      // TODO next group?
      nextCycle(song, CycleEnd::JOIN_ESTABLISHED_NO_LOOP);
      break;
    }
  }

private:
  // TODO cycle types (song or movement), depends on tuning and affects love
  inline float smoothValue(float current, float old) {
    float lambda = this->love_resolution / 44100;
    return old + (current - old) * lambda;
  }

  inline bool checkIfCycleInGroupOneIsObservedByCycleInGroupTwo(Group *one, Group *two) {
    if (!one) {
      return false;
    }

    if (one == two) {
      return true;
    }

    return checkIfCycleInGroupOneIsObservedByCycleInGroupTwo(one->parent_group, two);
  }

  inline void updateSongCyclesLove(std::vector<Cycle*> &cycles) {
    if (_love_calculator_divider.process()) {
      for (unsigned int i = 0; i < cycles.size(); i++) {
        float cycle_i_love = 1.f;
        for (unsigned int j = i + 1; j < cycles.size(); j++) {
          if (checkIfCycleInGroupOneIsObservedByCycleInGroupTwo(cycles[i]->group, cycles[j]->group)) {
            cycle_i_love *= (1.f - cycles[j]->readLove());
            if (cycle_i_love <= 0.001f) {
              cycle_i_love = 0.f;
              break;
            }
          }
        }

        _next_cycles_love[i] = cycle_i_love;
      }
    }

    for (unsigned int i = 0; i < cycles.size(); i++) {
      cycles[i]->love = smoothValue(_next_cycles_love[i], cycles[i]->love);
    }
  }

  inline void updateSongOut(Outputs &out, std::vector<Cycle*> cycles, Group* new_cycle_group, Inputs inputs) {
    // TODO get rid of me
    kokopellivcv::dsp::SignalType signal_type = kokopellivcv::dsp::SignalType::AUDIO;

    out.sun = 0.f;
    out.established = 0.f;

    for (unsigned int i = 0; i < cycles.size(); i++) {
      float cycle_out = cycles[i]->readSignal();
      if (checkIfCycleInGroupOneIsObservedByCycleInGroupTwo(cycles[i]->group, new_cycle_group)) {
        out.established = kokopellivcv::dsp::sum(out.established, cycle_out, signal_type);
        cycle_out *= (1.f - inputs.love);
      }

      out.sun = kokopellivcv::dsp::sum(out.sun, cycle_out, signal_type);
    }

    out.sun = kokopellivcv::dsp::sum(out.sun, inputs.in, signal_type);
  }

  inline void handleLoveDirectionChange(Song &song, LoveDirection new_love_direction) {
    assert(new_love_direction != _love_direction);

    if (new_love_direction == LoveDirection::ESTABLISHED) {
      nextCycle(song, CycleEnd::JOIN_ESTABLISHED_LOOP);
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

    updateSongCyclesLove(song.cycles);
    updateSongOut(song.out, song.cycles, song.new_cycle->group, inputs);
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
