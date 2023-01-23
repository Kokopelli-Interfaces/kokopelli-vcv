#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;
using namespace kokopellivcv::dsp;

void Engine::toggleMovementProgression() {
  _gko.conductor.loop_movement = !_gko.conductor.loop_movement;
}

void Engine::step() {
  _gko.advance(_song, inputs, options);
}

int Engine::getMostRecentCycleLength() {
  Cycle* recent_cycle = _song.cycles[_song.cycles.size()-1];
  return recent_cycle->immediate_group->convertToBeat(recent_cycle->period, false);
}

void Engine::cycleBackward() {
  // printf("Cycling backward\n");
  _gko.cycleBackward(_song);
}

void Engine::cycleForward() {
  _gko.cycleForward(_song);
}

void Engine::cycleObservation() {
  _gko.cycleObservation(_song);
}

bool Engine::isRecording() {
  return _gko._love_direction != LoveDirection::OBSERVED_SUN;
}

void Engine::ascend() {
  _gko.observer.ascend(_song);
  _gko.nextCycle(_song, CycleEnd::DISCARD);
  if (isRecording()) {
    _gko._discard_cycle_at_next_love_return = true;
  }
}

void Engine::undo() {
  _gko.undoCycle(_song);
}

void Engine::channelStateReset() {
  _gko.nextCycle(_song, CycleEnd::DISCARD);
  _gko.love_updater._love_calculator_divider.reset();
}
