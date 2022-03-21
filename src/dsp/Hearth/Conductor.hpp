#pragma once

#include "Village.hpp"
#include "Movement.hpp"
#include "definitions.hpp"

namespace kokopellivcv {
namespace dsp {
namespace circle {

// TODO renam eto grup???
struct Conductor {
  // TODO long press third button
  bool loop_movement = true;

  Conductor() {
    return;
  }

  bool endOfMovement(Village &village) {
    // village.current_movement->getMovementPhase();
    return false;
  }

  void progressToMovement(Village &village, Movement *next_movement) {
    for (auto cycle : next_movement->cycles_in_movement) {
      bool new_cycle = find(village.cycles.begin(), village.cycles.end(), cycle) == village.cycles.end();
      if (new_cycle) {
        cycle->playhead = 0.f;
        cycle->fader.fadeIn();
      }
    }

    for (auto cycle : village.cycles) {
      bool old_cycle = find(next_movement->cycles_in_movement.begin(), next_movement->cycles_in_movement.end(), cycle) == next_movement->cycles_in_movement.end();
      if (old_cycle) {
        cycle->fader.fadeOut();
      }
    }
  }

  void prevMovement(Village &village) {
    if (village.current_movement->prev) {
      village.current_movement = village.current_movement->prev;
    }
  }

  void nextMovement(Village &village) {
    if (village.current_movement->next) {
      village.current_movement = village.current_movement->next;
    } else {
      village.current_movement->next = new Movement(*village.current_movement);
      village.current_movement->next->prev = village.current_movement;
      village.current_movement->next->group_movement_n++;
      village.current_movement = village.current_movement->next;
    }
  }

  void conduct(Village &village) {
    if (endOfMovement(village)) {
      if (loop_movement) {
        return;
      } else if (village.current_movement->next) {
        progressToMovement(village, village.current_movement->next);
      }
    }
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
