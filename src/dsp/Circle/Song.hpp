#pragma once

#include "Cycle.hpp"

#include "dsp/PhaseAnalyzer.hpp"
#include "dsp/PhaseOscillator.hpp"
#include "definitions.hpp"
#include "util/math.hpp"
#include <vector>

namespace kokopellivcv {
namespace dsp {
namespace circle {

/**
   The Song is the top level structure for content.
*/
struct Song {
  std::string name = "My Song";
  std::vector<Cycle*> cycles;
  Section* start_section = nullptr;

  bool use_ext_phase = false;
  float ext_phase = 0.f;
  float sample_time = 1.0f;

  float love_resolution = 10000.f;

  TimePosition position;

  /** read only */

  PhaseOscillator _phase_oscillator;
  PhaseAnalyzer _phase_analyzer;

  rack::dsp::ClockDivider _love_calculator_divider;
  std::vector<float> _next_love;
  unsigned int _n_loved_cycles = 0;

  Song() {
    _love_calculator_divider.setDivision(10000);
  }

  inline bool phaseDefined() {
    return this->use_ext_phase || _phase_oscillator.isSet();
  }

  inline PhaseAnalyzer::PhaseEvent advanceSongPosition() {
    float internal_phase = _phase_oscillator.step(this->sample_time);
    this->position.phase = this->use_ext_phase ? this->ext_phase : internal_phase;

    PhaseAnalyzer::PhaseEvent phase_event = _phase_analyzer.process(this->position.phase, this->sample_time);

    unsigned int new_beat = this->position.beat;
    if (phase_event == PhaseAnalyzer::PhaseEvent::BACKWARD && 1 <= this->position.beat) {
      new_beat = this->position.beat - 1;
    } else if (phase_event == PhaseAnalyzer::PhaseEvent::FORWARD) {
      new_beat = this->position.beat + 1;
    }

    this->position.beat = new_beat;

    return phase_event;
  }


  inline int getSamplesPerBeat() {
    int samples_per_beat = 0;
    if (this->phaseDefined()) {
      if (this->use_ext_phase) {
        samples_per_beat = _phase_analyzer.getSamplesPerBeat(this->sample_time);
      } else if (_phase_oscillator.isSet()) {
        float beat_period = 1 / _phase_oscillator.getFrequency();
        samples_per_beat = floor(beat_period / this->sample_time);
      }
    }

    return samples_per_beat;
  }

  // TODO cycle types (song or section), depends on tuning and affects love
  inline void addCycle(Cycle* cycle) {
    // FIXME phase oscillators per group
    if (!_phase_oscillator.isSet()) {
      if (this->use_ext_phase && _phase_analyzer.getDivisionPeriod() != 0) {
        _phase_oscillator.setFrequency(1 / _phase_analyzer.getDivisionPeriod());
      } else {
        float recording_time = cycle->_signal->_samples_per_beat * this->sample_time;
        _phase_oscillator.setFrequency(1 / recording_time);
      }
      // printf("-- phase oscillator set with frequency: %f, sample time is: %f\n", _phase_oscillator.getFrequency(), this->sample_time);
    }

    cycles.push_back(cycle);
    _next_love.resize(cycles.size());
  }

  inline float smoothValue(float current, float old) {
    float lambda = this->love_resolution / 44100;
    return old + (current - old) * lambda;
  }

  inline bool atEnd(TimePosition position) {
    if (cycles.size() == 0) {
      return true;
    }

    unsigned int last_cycle_i = cycles.size()-1;
    return cycles[last_cycle_i]->_section->start_beat + cycles[last_cycle_i]->_n_beats <= position.beat + 1;
  }

  inline void updateCycleLove(TimePosition position) {
    if (_love_calculator_divider.process()) {
      unsigned int n_loved = 0;
      for (unsigned int cycle_i = 0; cycle_i < cycles.size(); cycle_i++) {

        float cycle_i_love = 1.f;
        for (unsigned int j = cycle_i + 1; j < cycles.size(); j++) {
          cycle_i_love -= cycles[j]->readLove(position);
          if (cycle_i_love <= 0.f)  {
            cycle_i_love = 0.f;
            break;
          }
        }

        if (0.f < cycle_i_love) {
          n_loved++;
        }

        _next_love[cycle_i] = cycle_i_love;
      }

      _n_loved_cycles = n_loved;
    }

    for (unsigned int i = 0; i < cycles.size(); i++) {
      cycles[i]->love = smoothValue(_next_love[i], cycles[i]->love);
    }
  }

  inline float read() {
    updateCycleLove(position);

    // FIXME
    kokopellivcv::dsp::SignalType signal_type = kokopellivcv::dsp::SignalType::AUDIO;

    float signal_out = 0.f;
    for (unsigned int i = 0; i < cycles.size(); i++) {
      float cycle_out = cycles[i]->listen(position);
      signal_out = kokopellivcv::dsp::sum(signal_out, cycle_out, signal_type);
    }

    return signal_out;
  }

  inline void undoCycle() {
    if (0 < cycles.size()) {
      cycles.erase(cycles.end()-1);
    }

    if (cycles.size() == 0) {
      _phase_oscillator.reset(0.f);
    }
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
