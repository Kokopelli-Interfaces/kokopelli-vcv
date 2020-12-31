#pragma once

#include "dsp/PhaseAnalyzer.hpp"
#include "Interface.hpp"
#include "dsp/PhaseOscillator.hpp"

namespace myrisa {
namespace dsp {
namespace gko {

class TimePositionAdvancer {
public:
  /** read only */
  PhaseOscillator _phase_oscillator;
  PhaseAnalyzer _phase_analyzer;

public:
  inline bool isPhaseOscillatorSet() {
    return _phase_oscillator.isSet();
  }

  inline bool phaseDefined(bool use_ext_phase) {
    return use_ext_phase || _phase_oscillator.isSet();
  }

  inline int getSamplesPerBeat() {
    return _phase_analyzer.getSamplesPerBeat()
  }

  inline int getBeatPeriod() {
    return _phase_analyzer.getBeatPeriod();
  }

  inline float setPhaseOscillatorFrequency() {
    if (!_phase_oscillator.isSet()) {
      if (_use_ext_phase && _phase_analyzer.getBeatPeriod() != 0) {
        _phase_oscillator.setFrequency(1 / _phase_analyzer.getBeatPeriod());
      } else {
        float recording_time = _recording_layer->_in->_samples_per_beat * _sample_time;
        _phase_oscillator.setFrequency(1 / recording_time);
      }
      printf("-- phase oscillator set with frequency: %f, sample time is: %f\n", _phase_oscillator.getFrequency(), _sample_time);
    }
  }

  inline void step(TimePosition &timeline_position, const Interface &interface) {
    PhaseAnalyzer::PhaseEvent phase_event = PhaseAnalyzer::PhaseEvent::NONE;

    if (phase_defined(interface.use_ext_phase, _phase_oscillator)) {
      float internal_phase = _phase_oscillator.step(interface.sample_time);
      timeline_position.phase = interface.use_ext_phase ? interface.ext_phase : internal_phase;

      phase_event = _phase_analyzer.process(timeline_position.phase, interface.sample_time);
      if (phase_event == PhaseAnalyzer::PhaseEvent::BACKWARD && 1 <= timeline_position.beat) {
        timeline_position.beat--;
      } else if (phase_event == PhaseAnalyzer::PhaseEvent::FORWARD) {
        timeline_position.beat++;
      }
    }

    return phase_event
  }
};

} // namespace gko
} // namespace dsp
} // namepsace myrisa
