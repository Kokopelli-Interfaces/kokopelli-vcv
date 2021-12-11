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
void Engine::cycleObservation() {
  _gko.cycleObservation(_song);
}

void Engine::ascend() {
  _gko.observer.ascend(_song);
  _gko.nextCycle(_song, CycleEnd::DISCARD);
}

void Engine::undo() {
  _gko.undoCycle(_song);
}

void Engine::channelStateReset() {
  _gko.nextCycle(_song, CycleEnd::DISCARD);
  _gko.love_updater._love_calculator_divider.reset();
}
