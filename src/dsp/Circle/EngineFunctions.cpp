#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;
using namespace kokopellivcv::dsp;

// FIXME
float Engine::getPhaseOfEstablished() {
  float length = _current_section->end_beat - _current_section->start_beat;
  float relative_position = this->song.position.beat - _current_section->start_beat + this->song.position.phase;
  return rack::clamp(relative_position / length, 0.f, 1.f);
}

int Engine::getMostRecentLoopLength() {
  for (int cycle_i = this->song.cycles.size()-1; cycle_i >= 0; cycle_i--) {
    if (this->song.cycles[cycle_i]->_loop) {
      return this->song.cycles[cycle_i]->_n_beats;
    }
  }

  return -1;
}

void Engine::deleteCycle(unsigned int cycle_i) {
}

// TODO FIXME
void Engine::nextGroup() {
}

bool Engine::isRecording() {
  return _new_cycle != nullptr;
}

void Engine::toggleTuneToFrequencyOfEstablished() {
  _tune_to_frequency_of_established = !_tune_to_frequency_of_established;
  // switch(_love_direction) {
  // case LoveDirection::ESTABLISHED:
  //   _tune_to_frequency_of_established = !_tune_to_frequency_of_established;
  //   break;
  // case LoveDirection::EMERGENCE:
  //   // if (_tune_to_frequency_of_established) {
  //     // this->nextCycle(CycleEnd::EMERGE_WITH_SECTION);
  //   // } else {
  //     _tune_to_frequency_of_established = true;
  //   // }
  //   break;
  // case LoveDirection::NEW:
  //   // TODO
  //   _tune_to_frequency_of_established = false;
  //   break;
  // }
}

void Engine::progress() {
  switch(_love_direction) {
  case LoveDirection::ESTABLISHED:
    if (_tune_to_frequency_of_established) {
      this->nextCycle(CycleEnd::DISCARD_AND_NEXT_SECTION_IN_GROUP);
    } else {
      this->nextCycle(CycleEnd::DISCARD_AND_NEXT_SECTION);
      // A -> B
      // TODO should it place the recording ?
      // this->nextCycle(false);
    }
    break;
  case LoveDirection::EMERGENCE:
    if (_tune_to_frequency_of_established) {
      this->nextCycle(CycleEnd::EMERGE_WITH_SECTION_AND_CREATE_NEXT_SECTION);
    } else {
      this->nextCycle(CycleEnd::DO_NOT_LOOP_AND_NEXT_SECTION);
    }
    break;
  case LoveDirection::NEW:
    // this->nextCycle(CycleEnd::DO_NOT_LOOP_AND_NEXT_SECTION);
    // TODO
    this->nextCycle(SET_TO_SONG_PERIOD_AND_NEXT_GROUP);
    break;
  }
}

// TODO
void Engine::ascend() {
  // this->nextCycle(CycleEnd::DISCARD);


  // switch(_love_direction) {
  // case LoveDirection::ESTABLISHED:
  //   if (_tune_to_frequency_of_established) {
  //     if (_cycle_mode) {
  //       // ??? cycle focused_cycle (displays on NEW in purple)
  //     } else {
  //       // cycle focused_group (displays on NEW in purple)
  //       // use to  enter
  //     }
  //   }
  //   break;
  // case LoveDirection::EMERGENCE:
  //   if (_tune_to_frequency_of_established) {
  //     // TODO CREATE WINDOW RECORDING WITH SIZE
  //     // this->nextCycle(CycleEnd::)
  //   } else {
  //     this->nextCycle(CycleEnd::EMERGETHIS->SONG_AND_NEXT_SECTION);;
  //   }
  //   break;
  // case LoveDirection::NEW:
  //   // TODO
  //   // this->nextCycle(CycleEnd::NEXT_GROUP);
  //   break;
  // }
}

void Engine::descend() {
  // TODO A  ( )  1   ->  1  ( )  I
  // start recording in cycle that loves cycles that are also focused on cycle 1
  // the cycle becomes the ESTABLISHED.
}

void Engine::regress() {
  this->song.undoCycle();
  this->nextCycle(CycleEnd::DISCARD);
}

// FIXME
void Engine::toggleCycleMode() {
}
