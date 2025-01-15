#pragma once

#include <vector>
#include <numeric> // std::iota

#include "rack.hpp"

#include "definitions.hpp"
#include "util/math.hpp"

namespace kokopellivcv {
namespace dsp {
namespace circle {

class Conductor {
public:
  bool progression_mode = false;

  // read only
  int _movement_i = -1;

private:
  inline bool allCyclesHaveDifferentMovements(std::vector<Cycle*> &cycles) {
    if (cycles.empty()) {
      return true;
    }

    for (unsigned int i = 0; i < cycles.size(); i++) {
      for (unsigned int j = i + 1; j < cycles.size(); j++) {
        if (cycles[i]->enter_at_movement_i == cycles[j]->enter_at_movement_i) {
          return false;
        }
      }
    }

    return true;
  }

public:
  inline void createNewMovementForCycleAndShiftMovementsAfter(std::vector<Cycle*> &cycles, Cycle* new_cycle) {
    for (unsigned int i = 0; i < cycles.size(); i++) {
      if (cycles[i] == new_cycle) {
        assert(i == cycles.size() - 1);
        continue;
      }

      if (new_cycle->enter_at_movement_i <= cycles[i]->enter_at_movement_i) {
        cycles[i]->enter_at_movement_i++;
      }
    }
  }

  // cycle must be removed from cycles afterwards
  inline void deleteCycleFromCyclesAndAdjustMovements(std::vector<Cycle*> &cycles, int cycle_i) {
    int cycle_to_erase_movement = cycles[cycle_i]->enter_at_movement_i;
    for (unsigned int i = 0; i < cycles.size(); i++) {
      if (cycles[cycle_i]->enter_at_movement_i < cycles[i]->enter_at_movement_i ) {
        cycles[i]->enter_at_movement_i--;
      }
    }
              cycles[cycle_i]->immediate_group->undoLastCycle();
              cycles.erase(cycles.begin() + cycle_i);

    if (cycle_to_erase_movement <= _movement_i) {
      if (_movement_i != 0) {
        progressToMovement(cycles, _movement_i-1);
      }
    }
  }

  inline void progressToMovement(std::vector<Cycle*> &cycles, int next_movement_i) {
    assert(allCyclesHaveDifferentMovements(cycles));

    // skip flags are used to bring in multiple cycles at once
    for (unsigned int i = 0; i < cycles.size(); i++) {
      cycles[i]->is_playing = false;
      if (cycles[i]->enter_at_movement_i <= next_movement_i) {
        cycles[i]->is_playing = true;
      }
    }

    _movement_i = next_movement_i;
  }

  inline int getNextMovementI(std::vector<Cycle*> &cycles, bool ignore_cycle_skip_flag) {
    int next_movement_i = _movement_i;
    bool potential_next_movement = false;

    for (unsigned int i = 0; i < cycles.size(); i++) {
      if (cycles[i]->skip_in_progression && !ignore_cycle_skip_flag) {
        continue;
      }

      if (_movement_i < cycles[i]->enter_at_movement_i) {
        if (!potential_next_movement) {
          next_movement_i = cycles[i]->enter_at_movement_i;
          potential_next_movement = true;
        } else {
          next_movement_i = std::min(next_movement_i, cycles[i]->enter_at_movement_i);
        }
      }
    }

    return next_movement_i;
  }

  inline void nextMovement(std::vector<Cycle*> &cycles, bool ignore_cycle_skip_flag) {
    int next_movement_i = getNextMovementI(cycles, ignore_cycle_skip_flag);

    if (next_movement_i != _movement_i) {
      progressToMovement(cycles, next_movement_i);
    }
  }

  inline int getPrevMovementI(std::vector<Cycle*> &cycles, bool ignore_cycle_skip_flag) {
    int prev_movement_i = _movement_i;
    bool potential_next_movement = false;

    for (unsigned int i = 0; i < cycles.size(); i++) {
      if (cycles[i]->skip_in_progression && !ignore_cycle_skip_flag) {
        continue;
      }

      if (cycles[i]->enter_at_movement_i < _movement_i ) {
        if (!potential_next_movement) {
          prev_movement_i = cycles[i]->enter_at_movement_i;
          potential_next_movement = true;
        } else {
          prev_movement_i = std::max(prev_movement_i, cycles[i]->enter_at_movement_i);
        }
      }
    }

    return prev_movement_i;
  }

  inline void prevMovement(std::vector<Cycle*> &cycles, bool ignore_cycle_skip_flag) {
    int prev_movement_i = getPrevMovementI(cycles, ignore_cycle_skip_flag);

    if (prev_movement_i != _movement_i) {
      progressToMovement(cycles, prev_movement_i);
    }
  }

  inline void goToStartMovement(std::vector<Cycle*> &cycles) {
    int start_movement_i = 0;
    if (_movement_i != start_movement_i) {
      progressToMovement(cycles, start_movement_i);
    }
  }

  inline void toggleSkipOnCycleAtCurrentMovement(std::vector<Cycle*> &cycles) {
    assert(allCyclesHaveDifferentMovements(cycles));
    for (unsigned int i = 0; i < cycles.size(); i++) {
      if (cycles[i]->enter_at_movement_i == _movement_i) {
        cycles[i]->skip_in_progression = !cycles[i]->skip_in_progression;
      }
    }
  }

  inline bool isCurrentMovementSkipped(std::vector<Cycle*> &cycles) {
    assert(allCyclesHaveDifferentMovements(cycles));
    for (unsigned int i = 0; i < cycles.size(); i++) {
      if (cycles[i]->enter_at_movement_i == _movement_i) {
        return cycles[i]->skip_in_progression;
      }
    }

    return false;
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
