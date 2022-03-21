#pragma once

#include "Village.hpp"
#include "Movement.hpp"
#include "definitions.hpp"

namespace kokopellivcv {
namespace dsp {
namespace hearth {

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
    for (auto voice : next_movement->voices_in_movement) {
      bool new_voice = find(village.voices.begin(), village.voices.end(), voice) == village.voices.end();
      if (new_voice) {
        voice->playhead = 0.f;
        voice->fader.fadeIn();
      }
    }

    for (auto voice : village.voices) {
      bool old_voice = find(next_movement->voices_in_movement.begin(), next_movement->voices_in_movement.end(), voice) == next_movement->voices_in_movement.end();
      if (old_voice) {
        voice->fader.fadeOut();
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

} // namespace hearth
} // namespace dsp
} // namespace kokopellivcv
