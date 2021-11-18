#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;
using namespace kokopellivcv::dsp;

void Engine::step() {
  _gko.advance(_song, inputs);
}

int Engine::getMostRecentCycleLength() {
  Cycle* recent_cycle = _song.cycles[_song.cycles.size()-1];
  return recent_cycle->immediate_group->convertToBeat(recent_cycle->period, false);
}

void Engine::toggleTuneToFrequencyOfEstablished() {
  _gko.tune_to_frequency_of_established = !_gko.tune_to_frequency_of_established;
}

void Engine::cycleForward() {
  _gko.cycleForward(_song);
}

// TODO
void Engine::cycleDivinity() {
  _gko.cycleDivinity(_song);
  // TODO make a group !
}

void Engine::ascend() {
  _gko.ascend(_song);
}

void Engine::undo() {
  _gko.undoCycle(_song);
  _gko.nextCycle(_song, CycleEnd::DISCARD);
}
