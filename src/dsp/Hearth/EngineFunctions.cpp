#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;
using namespace kokopellivcv::dsp;

void Engine::toggleMovementProgression() {
  _gko.conductor.loop_movement = !_gko.conductor.loop_movement;
}

void Engine::step() {
  _gko.advance(_village, inputs, options);
}

int Engine::getMostRecentCycleLength() {
  Cycle* recent_cycle = _village.cycles[_village.cycles.size()-1];
  return recent_cycle->immediate_group->convertToBeat(recent_cycle->period, false);
}

void Engine::cycleBackward() {
  printf("Cycling backward\n");
  _gko.cycleBackward(_village);
}

void Engine::cycleForward() {
  _gko.cycleForward(_village);
}

void Engine::cycleObservation() {
  _gko.cycleObservation(_village);
}

void Engine::ascend() {
  _gko.observer.ascend(_village);
  _gko.nextCycle(_village, CycleEnd::DISCARD);
  _gko._discard_cycle_at_next_love_return = true;
}

void Engine::undo() {
  _gko.undoCycle(_village);
}

void Engine::channelStateReset() {
  _gko.nextCycle(_village, CycleEnd::DISCARD);
  _gko.love_updater._love_calculator_divider.reset();
}
