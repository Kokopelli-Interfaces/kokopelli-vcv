#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;
using namespace kokopellivcv::dsp;

void Engine::step() {
  _gko.advance(_song, inputs);
}

int Engine::getMostRecentCycleLength() {
  for (int cycle_i = _song.cycles.size()-1; cycle_i >= 0; cycle_i--) {
    return _song.cycles[cycle_i]->period.tick;
  }

  return -1;
}

void Engine::toggleTuneToFrequencyOfEstablished() {
  _gko.tune_to_frequency_of_established = !_gko.tune_to_frequency_of_established;
}

void Engine::cycleForward() {
  _gko.cycleForward(_song);
}

// TODO
void Engine::ascend() {
}

void Engine::descend() {
  // TODO A  ( )  1   ->  1  ( )  I
  // start recording in cycle that loves cycles that are also focused on cycle 1
  // the cycle becomes the ESTABLISHED.
}

void Engine::undo() {
  _gko.undoCycle(_song.cycles);
  _gko.nextCycle(_song, CycleEnd::DISCARD);
}
