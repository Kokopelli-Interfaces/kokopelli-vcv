#pragma once

#include "Village.hpp"
#include "definitions.hpp"

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct Cycle;

struct Movement {
  char timeline = '*';
  int group_movement_n = 1;

  std::vector<Cycle*> cycles_in_movement;

  Movement *group_start_movement = nullptr;
  Movement *prev = nullptr;
  Movement *next = nullptr;

  // TODO
  // inline float getMovementPhase(Time song_time) {
  //   float movement_phase = 0.f;
  //   if (this->next) {
  //     Time relative_time = song_time - this->start;
  //     Time period = this->next->start - this->start;
  //     movement_phase = relative_time / period;
  //   }
  //   return rack::clamp(movement_phase, 0.f, 1.f);
  // }

  // inline static Movement* createNextMovementWithNewGroup(Movement* movement, unsigned int movement_period) {
  //   Movement* group_movement = createNextMovement(movement, start_beat);
  //   group_movement->group++;
  //   group_movement->group_movement_n = 1;
  //   group_movement->group_start_movement = group_movement;
  //   group_movement->period = movement_period;
  //   return group_movement;
  // }

  // inline static Movement* createNextMovement(Movement* movement, Time movement_period) {
  //   assert(movement);

  //   Movement *new_movement = new Movement();
  //   *new_movement = *movement;

  //   new_movement->group_movement_n++;

  //   Movement* future_movement = movement->next;
  //   while (future_movement) {
  //     if (future_movement->group == movement->group)  {
  //       future_movement->group_movement_n++;
  //     }
  //     future_movement = future_movement->next;
  //   }

  //   movement->next = new_movement;
  //   new_movement->prev = movement;

  //   new_movement->period = movement_period;

  //   return new_movement;
  // }

  // inline static Movement* findNextMovementWithSameGroup(Movement* movement) {
  //   assert(movement);

  //   Movement* future_movement = movement->next;
  //   while (future_movement) {
  //     if (future_movement->group == movement->group)  {
  //       break;
  //     }
  //     future_movement = future_movement->next;
  //   }

  //   if (future_movement == nullptr) {
  //     assert(movement->group_start_movement);
  //     return movement->group_start_movement;
  //   }

  //   return future_movement;
  // }

  // // TODO should it be this way? or different lists for each group
  // //                    B1->B2
  // // like    A1->A2->A3      ^C1-----
  // //        A^--------->B^---C-------
  // inline static Movement* findNextMovement(Movement* movement) {
  //   if (movement->next) {
  //     return movement->next;
  //   } else {
  //     Movement* past_movement = movement;
  //     while (past_movement) {
  //       if (!past_movement->prev) {
  //         return past_movement;
  //       }
  //       past_movement = past_movement->prev;
  //     }

  //     assert(true == false);
  //     return nullptr;
  //   }
  // }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
