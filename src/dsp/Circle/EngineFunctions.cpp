#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;
using namespace kokopellivcv::dsp;

void Engine::step() {
  _gko.advance(_song, inputs, options);
}

int Engine::getMostRecentCycleLength(bool in_observed_sun) {
  if (in_observed_sun) {
    if (_song.observed_sun && !_song.observed_sun->cycles_in_group.empty()) {
      Cycle* last_cycle = _song.observed_sun->cycles_in_group.back();
      return _song.observed_sun->convertToBeat(last_cycle->_period, false);
    }

    return 0;
  }

  Cycle* recent_cycle = _song.cycles[_song.cycles.size()-1];
  return recent_cycle->immediate_group->convertToBeat(recent_cycle->_period, false);
}

void Engine::cycleForward() {
  _gko.cycleForward(_song, options);
}

void Engine::cycleObservation() {
  _gko.cycleObservation(_song, options.cycle_forward_not_back);
}

void Engine::prevMovement(bool ignore_skip_flag) {
  _gko.conductor.prevMovement(_song.cycles, ignore_skip_flag);
}

void Engine::nextMovement(bool ignore_skip_flag) {
  _gko.conductor.nextMovement(_song.cycles, ignore_skip_flag);
}

void Engine::goToStartMovement() {
  _gko.conductor.goToStartMovement(_song.cycles);
}

void Engine::toggleProgression() {
  _gko.toggleProgression(_song);
}

void Engine::toggleSkip() {
  if (isRecording()) {
    _song.new_cycle->skip_in_progression = !_song.new_cycle->skip_in_progression;
  } else {
    _gko.conductor.toggleSkipOnCycleAtCurrentMovement(_song.cycles);
  }
}

bool Engine::isRecording() {
  return _gko._love_direction != LoveDirection::OBSERVED_SUN;
}

bool Engine::isInProgressionMode() {
  return _gko.conductor.progression_mode;
}

void Engine::ascend() {
  _gko.observer.ascend(_song);
  _gko.nextCycle(_song, CycleEnd::DISCARD);
  if (isRecording()) {
    _gko._discard_cycle_at_next_love_return = true;
  }
}

void Engine::deleteCycles() {
  _gko.deleteCycles(this->_song, this->options);
}

void Engine::channelStateReset() {
  _gko.nextCycle(_song, CycleEnd::DISCARD);
  _gko.love_updater._love_calculator_divider.reset();
}

float Engine::getPhaseInObservedSong() {
  if (this->_gko.use_ext_phase && this->_song.cycles.empty()) {
    return this->_gko.ext_phase;
  }

  // TODO next cycles may create a progression period
  // if (isInProgressionMode()) {
  // }

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

int Engine::getMostRecentCycleInObservedSongBeat() {
  if (_song.observed_sun && !_song.observed_sun->cycles_in_group.empty()) {
    Cycle* last_cycle = _song.observed_sun->cycles_in_group.back();
    Time last_cycle_playhead = last_cycle->playhead - last_cycle->_offset_in_group;
    return _song.observed_sun->convertToBeat(last_cycle_playhead, true);
  }
  return 0;
}

int Engine::getNumberOfCyclesInGroupAccountingForCurrentMovement() {
  return 0;
}
