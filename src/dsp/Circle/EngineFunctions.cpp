#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;
using namespace kokopellivcv::dsp;

void Engine::step() {
  _gko.advance(_song, inputs, options);
}

int Engine::getMostRecentCycleLength() {
  Cycle* recent_cycle = _song.cycles[_song.cycles.size()-1];
  return recent_cycle->immediate_group->convertToBeat(recent_cycle->period, false);
}

void Engine::cycleForward() {
  _gko.cycleForward(_song, options);
}

void Engine::cycleObservation() {
  _gko.cycleObservation(_song, options.cycle_forward_not_back);
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

float Engine::getPhaseInObservedSun() {
  if (this->_gko.use_ext_phase && this->_song.cycles.empty()) {
    return this->_gko.ext_phase;
  }

  return this->_song.observed_sun->getPhase(this->_song.playhead);
}

float Engine::getBeatPhase() {
  if (this->_gko.use_ext_phase && this->_song.cycles.empty()) {
    return this->_gko.ext_phase;
  }

  if (this->_song.groups.empty()) {
    return 0.f;
  }

  if (this->options.output_observed_song_beat_phase_not_total_song) {
    return this->_song.observed_sun->getBeatPhase(this->_song.playhead);
  }

  return this->_song.groups[0]->getBeatPhase(this->_song.playhead);
}
