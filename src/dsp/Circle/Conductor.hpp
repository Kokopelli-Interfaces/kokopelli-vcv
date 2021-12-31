#pragma once

#include "Song.hpp"
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

  inline void play(Cycle *cycle) {
    cycle->playhead = 0.f;
  }

  inline void fadeIn(Cycle *cycle) {
    return;
  }

  inline void fadeOut(Cycle *cycle) {
    return;
  }

  bool endOfMovement(Song &song) {
    // song.current_movement->getMovementPhase();
    return false;
  }

  void progressToMovement(Song &song, Movement *next_movement) {
    for (auto cycle : next_movement->cycles_in_movement) {
      bool new_cycle = find(song.cycles.begin(), song.cycles.end(), cycle) == song.cycles.end();
      if (new_cycle) {
        play(cycle);
        fadeIn(cycle);
      }
    }

    for (auto cycle : song.cycles) {
      bool old_cycle = find(next_movement->cycles_in_movement.begin(), next_movement->cycles_in_movement.end(), cycle) == next_movement->cycles_in_movement.end();
      if (old_cycle) {
        fadeOut(cycle);
      }
    }
  }

  void nextMovementViaShift(Song &song) {
    if (song.current_movement->next) {
      song.current_movement = song.current_movement->next;
    } else {
      song.current_movement->next = new Movement(*song.current_movement);
      song.current_movement->next->prev = song.current_movement;
      song.current_movement->next->group_movement_n++;

      song.current_movement = song.current_movement->next;
    }
  }

  void conduct(Song &song) {
    if (endOfMovement(song)) {
      if (loop_movement) {
        return;
      } else if (song.current_movement->next) {
        progressToMovement(song, song.current_movement->next);
      }
    }
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
