#include "Member.hpp"

using namespace kokopelli::dsp::circle;

static float smoothValue(float current, float old) {
  const float lambda = 30.f / 44100;
  return old + (current - old) * lambda;
}

void Member::updateMemberAttenuations(unsigned int beat, float phase) {
  if (_attenuation_calculator_divider.process()) {
    for (unsigned int member_i = 0; member_i < _members.size(); member_i++) {
      float member_i_attenuation = 0.f;
      for (unsigned int j = member_i + 1; j < _members.size(); j++) {
        for (auto target_member_i : _members[j]->members_being_observed_idx) {
          if (target_member_i == member_i) {
            member_i_attenuation += _members[j]->readPhaseBufferLove(beat, phase);
            break;
          }
        }

        if (1.f <= member_i_attenuation)  {
          member_i_attenuation = 1.f;
          break;
        }
      }

      _last_calculated_attenuation[member_i] = member_i_attenuation;
    }
  }

void Member::prevBeat() {
  for (auto member : _members) {
    member->prevBeat();
  }
}

void Member::nextBeat() {
}

float Member::listen(float in, float love) {
  updateMemberAttenuations(position);

  // FIXME multiple recordings in member, have loop and array of types
  // kokopelli::dsp::SignalType signal_type = kokopelli::dsp::SignalType::AUDIO;
  // if (0 < _members.size()) {
  //   signal_type = _members[0]->_in->_signal_type;
  // }

  float signal_out = 0.f;
  for (unsigned int i = 0; i < _members.size(); i++) {
    if (_members[i]->alive(position)) {
      float attenuation = _current_attenuation[i];

      if (record_params.active()) {
        for (unsigned int sel_i : recording->members_being_observed_idx) {
          if (sel_i == i) {
            attenuation += record_params.love;
            break;
          }
        }
      }

      attenuation = rack::clamp(attenuation, 0.f, 1.f);
      float member_out = _members[i]->sing(position);
      member_out = kokopelli::dsp::attenuate(member_out, attenuation, signal_type);
      if (i == active_member_i) {
        active_member_out = member_out;
      }

      signal_out = kokopelli::dsp::sum(signal_out, member_out, signal_type);
    }
  }

  return signal_out;
}

// void Member::advanceTime() {
//   float internal_phase = _phase_oscillator.step(interface->sample_time);
//   _phase = interface->_use_ext_phase ? interface->_ext_phase : internal_phase;

//   PhaseAnalyzer::PhaseEvent phase_event = _phase_analyzer.process(phase, interface->sample_time);

//   if (phase_event == PhaseAnalyzer::PhaseEvent::DISCONTINUITY && _options.use_antipop) {
//     _read_antipop_filter.trigger();
//   }

//   if (phase_event == PhaseAnalyzer::PhaseEvent::FORWARD) {
//     this->_cicle.nextBeat();
//   } else if (phase_event == PhaseAnalyzer::PhaseEvent::BACKWARD) {
//     this->_cicle.prevBeat();
//   }
// }

inline unsigned int Member::getNumberOfBeats(TimePosition position) {
  unsigned int max_n_beats = 0;
  for (auto member: _members) {
    if (member->alive() && max_n_beats < member->_n_beats) {
      max_n_beats = member->_n_beats;
    }
  }

  return max_n_beats;
}


float Member::advance(Interface *interface) {
  float phase = _phase_oscillator.step(interface->sample_time);
  // _phase = _use_ext_phase ? _ext_phase : internal_phase;

  PhaseAnalyzer::PhaseEvent phase_event = _phase_analyzer.process(phase, interface->sample_time);

  if (phase_event == PhaseAnalyzer::PhaseEvent::DISCONTINUITY && _options.use_antipop) {
    _read_antipop_filter.trigger();
  }

  if (phase_event == PhaseAnalyzer::PhaseEvent::FORWARD) {
    this->_cicle.nextBeat();
  } else if (phase_event == PhaseAnalyzer::PhaseEvent::BACKWARD) {
    this->_cicle.prevBeat();
  }


  for (auto member : _members) {
    member->advance();
  }


  void listen(float in, float love) {
    // assert(_in->_buffer.size() <= _n_beats);
    // assert(_love->_buffer.size() <= _n_beats);

    // if (!phase_defined) {

    _in->pushBack(in);
    _love->pushBack(love);

   //  } else if (writableAtPosition(timeline_position)) {
   //    _in->write(getPhaseBufferPosition(timeline_position), in);
   // _love->write(getPhaseBufferPosition(timeline_position), love);
   //  }
  }

void nextBeat() {
  _beat++;
  if (_n_beats <= _beat) {
    _beat = 0;
  }

  for (auto member : _circle) {
    member->nextBeat();
  }
}

bool alive() {
  if (_alive) {
    return true;
  }

  for (auto member : _circle) {
    if (member->_alive) {
      return true;
    }
  }

  return false;
}

float sing(float phase) {
  return _in->read(_beat, phase);
}

