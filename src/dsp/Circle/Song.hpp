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

struct Song {
  std::string name = "my song";

  std::vector<Group*> groups;
  Group* established_group;

  std::vector<Cycle*> cycles;
  Cycle *new_cycle = nullptr;

  Time playhead = 0.f;

  Movement *current_movement = nullptr;
  Movement* start_movement = nullptr;

  Outputs out;

  Song() {
    this->current_movement = new Movement();
    this->current_movement->group_start_movement = this->current_movement;
    this->start_movement = this->current_movement;
    Time start = 0.f;
    this->groups.push_back(new Group());
    this->established_group = groups[0];
    this->new_cycle = new Cycle(start, this->current_movement, this->established_group);
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
