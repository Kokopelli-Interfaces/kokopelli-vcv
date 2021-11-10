#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;
using namespace kokopellivcv::dsp;

inline int findNiceNumberAroundFirstThatFitsIntoSecond(int n1, int n2) {
  assert(n1 < n2);

  while (n2 % n1 != 0) {
    n1++;
    if (n1 == n2) {
      return n1;
    }
  }

  return n1;
}

inline void fitCycleIntoSection(Cycle* cycle, Section* section) {
  unsigned int n_section_beats = section->end_beat - section->start_beat;
  if (cycle->_n_beats == n_section_beats) {
    return;
  }

  bool grow_section = n_section_beats < cycle->_n_beats;
  if (grow_section) {
    unsigned int new_n_section_beats = n_section_beats;
    while (new_n_section_beats < cycle->_n_beats) {
      new_n_section_beats += n_section_beats;
    }
    cycle->_n_beats = new_n_section_beats;
    section->end_beat = section->start_beat + new_n_section_beats;
  } else {
    cycle->_n_beats = findNiceNumberAroundFirstThatFitsIntoSecond(cycle->_n_beats, n_section_beats);
  }
}

void Engine::nextCycle(CycleEnd cycle_end) {
  switch (cycle_end) {
  case CycleEnd::NO_CYCLE_ENDED:
    // TODO check if song is
    _current_section = new Section();
    _current_section->group_start_section = _current_section;
    song.start_section = _current_section;
  case CycleEnd::DISCARD:
    break;
  case CycleEnd::DISCARD_AND_NEXT_SECTION_IN_GROUP:
    _current_section = Section::findNextSectionWithSameGroup(_current_section);
    this->song.position.beat = _current_section->start_beat;
    break;
  case CycleEnd::DISCARD_AND_NEXT_SECTION:
    _current_section = Section::findNextSection(_current_section);
    this->song.position.beat = _current_section->start_beat;
    break;
  case CycleEnd::EMERGE_WITH_SECTION:
    _new_cycle->_loop = true;
    fitCycleIntoSection(_new_cycle, _current_section);
    this->song.addCycle(_new_cycle);
    break;
  // case CycleEnd::SET_PERIOD_TO_SECTION_AND_EMERGE_WITH_SECTION:
  //   _new_cycle->_loop = true;
  //   _new_cycle->updateCyclePeriod(_new_cycle->_section->end_beat - _new_cycle->_section->start_beat);
  //   this->song.addCycle(_new_cycle);
  //   break;
  case CycleEnd::EMERGE_WITH_SECTION_AND_CREATE_NEXT_SECTION:
    _new_cycle->_loop = true;
    fitCycleIntoSection(_new_cycle, _current_section);
    this->song.addCycle(_new_cycle);
    _current_section = Section::createNewSectionAfterSectionWithSameLength(_current_section);
    this->song.position.beat += _current_section->getNBeats();
    break;
  case CycleEnd::DO_NOT_LOOP_AND_NEXT_SECTION:
    // section may have changed already, in which case, just leave it
    _new_cycle->_loop = false;
    this->song.addCycle(_new_cycle);
    if (_new_cycle->_section->next == nullptr) {
      _current_section = Section::createNextSection(_current_section, this->song.position.beat);
    }
    break;
  case CycleEnd::SET_TO_SONG_PERIOD_AND_NEXT_GROUP:
    // TODO
    _new_cycle->_loop = false;
    this->song.addCycle(_new_cycle);
    if (_new_cycle->_section->next == nullptr) {
      _current_section = Section::createNextGroupSection(_current_section, this->song.position.beat);
    }
    break;
  }

  int samples_per_beat = this->song.getSamplesPerBeat();
  _new_cycle = new Cycle(_current_section, _signal_type, samples_per_beat);
}

inline void Engine::handleBeatChange(PhaseAnalyzer::PhaseEvent event) {
  assert(this->song.phaseDefined());

  // FIXME sections dont have ends, just beginnings
  bool reached_section_end =  _current_section->end_beat <= this->song.position.beat;
  if (reached_section_end) {
    if (!_tune_to_frequency_of_established && _current_section->next) {
      _current_section = _current_section->next;
    } else {
      // TODO doesnt work too well, use antipop trigger instead
      _read_antipop_filter.trigger();
      this->song.position.beat = _current_section->start_beat;
    }
  }

  // TODO cycles should handle this
  // TODO better way?
  if (event == PhaseAnalyzer::PhaseEvent::FORWARD) {
    _new_cycle->writeNextBeat();
  } else if (event == PhaseAnalyzer::PhaseEvent::BACKWARD) {
    _new_cycle->writePrevBeat();
  }
}

void Engine::step() {
  if (this->song.phaseDefined()) {
    PhaseAnalyzer::PhaseEvent phase_event = this->song.advanceSongPosition();
    if (phase_event == PhaseAnalyzer::PhaseEvent::DISCONTINUITY && this->options.use_antipop) {
      _read_antipop_filter.trigger();
    }

    if (phase_event == PhaseAnalyzer::PhaseEvent::FORWARD || phase_event == PhaseAnalyzer::PhaseEvent::BACKWARD) {
      this->handleBeatChange(phase_event);
    }
  }

  _new_cycle->write(this->song.position.phase, this->inputs.in, this->inputs.love, this->song.phaseDefined());

  LoveDirection new_direction = Inputs::getLoveDirection(this->inputs.love);
  if (_love_direction != new_direction) {
    if (_tune_to_frequency_of_established) {
      if (new_direction == LoveDirection::ESTABLISHED) {
        this->nextCycle(CycleEnd::EMERGE_WITH_SECTION);
      } else if (_love_direction == LoveDirection::ESTABLISHED) {
        this->nextCycle(CycleEnd::DISCARD);
      }
    }

    _love_direction = new_direction;
  }
}

Engine::Engine() {
  this->nextCycle(CycleEnd::NO_CYCLE_ENDED);
}
