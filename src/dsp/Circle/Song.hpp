#pragma once

#include "Cycle.hpp"
#include "Movement.hpp"
#include "Group.hpp"
#include "definitions.hpp"
#include "util/math.hpp"

#include <vector>

namespace kokopellivcv {
namespace dsp {
namespace circle {

/**
   The Song is the top level structure for content.
*/
struct Song {
  // struct Playhead {
  //   Movement* movement;
  //   unsigned int beat;
  //   float phase;
  // };

  std::string name = "my song";

  std::vector<Group> groups;
  Group* established_group;

  std::vector<Cycle*> cycles;

  Cycle *new_cycle = nullptr;

  Time playhead;

  Movement *current_movement = nullptr;
  Movement* start_movement = nullptr;

  Song() {
    this->current_movement = new Movement();
    this->current_movement->group_start_movement = this->current_movement;
    this->start_movement = this->current_movement;
    Time start;
    Group first_group;
    this->groups.push_back(first_group);
    this->established_group = &groups[0];
    this->new_cycle = new Cycle(start, this->current_movement, this->established_group);
  }

  inline float read() {
    kokopellivcv::dsp::SignalType signal_type = kokopellivcv::dsp::SignalType::AUDIO;

    float signal_out = 0.f;
    for (unsigned int i = 0; i < cycles.size(); i++) {
      float cycle_out = cycles[i]->readSignal();
      signal_out = kokopellivcv::dsp::sum(signal_out, cycle_out, signal_type);
    }

    return signal_out;
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
