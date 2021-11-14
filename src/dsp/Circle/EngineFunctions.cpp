#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;
using namespace kokopellivcv::dsp;

void Engine::step() {
  _gko.advance(_song, inputs);
}

int Engine::getMostRecentCycleLength() {
  return _song.cycles[_song.cycles.size()-1]->period.tick;
}

void Engine::toggleTuneToFrequencyOfEstablished() {
  _gko.tune_to_frequency_of_established = !_gko.tune_to_frequency_of_established;
}

void Engine::cycleForward() {
  _gko.cycleForward(_song);
}

// TODO
void Engine::ascend() {
  // TODO make a group !
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
