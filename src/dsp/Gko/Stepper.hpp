#pragma once

#include "dsp/PhaseAnalyzer.hpp"
#include "dsp/PhaseOscillator.hpp"

namespace myrisa {
namespace dsp {
namespace gko {

class Stepper {
public:
  /** read only */
  PhaseOscillator _phase_oscillator;
  PhaseAnalyzer _phase_analyzer;

public:
  inline bool isPhaseOscillatorSet() {
    return _phase_oscillator.isSet();
  }

  inline float getBeatPeriod() {
    if (!_phase_oscillator.isSet()) {
      if (_use_ext_phase && _phase_analyzer.getDivisionPeriod() != 0) {
        _phase_oscillator.setFrequency(1 / _phase_analyzer.getDivisionPeriod());
      } else {
        float recording_time = _recording_layer->_in->_samples_per_beat * _sample_time;
        _phase_oscillator.setFrequency(1 / recording_time);
      }
      printf("-- phase oscillator set with frequency: %f, sample time is: %f\n", _phase_oscillator.getFrequency(), _sample_time);
    }
  }

  inline void step(float sample_time, float ext_phase, bool use_ext_phase) {
    float internal_phase = _phase_oscillator.step(sample_time);
    _timeline_position.phase = use_ext_phase ? ext_phase : internal_phase;

    PhaseAnalyzer::PhaseEvent phase_event = _phase_analyzer.process(_timeline_position.phase, sample_time);
    if (phase_event == PhaseAnalyzer::PhaseEvent::BACKWARD && 1 <= _timeline_position.beat) {
      _timeline_position.beat--;
    } else if (phase_event == PhaseAnalyzer::PhaseEvent::FORWARD) {
      _timeline_position.beat++;
    }

    return phase_event;
  }

  void step() {
    bool phase_defined = this->phaseDefined();

    if (phase_defined) {
      PhaseAnalyzer::PhaseEvent phase_event = this->advanceTimelinePosition();
      if (phase_event != PhaseAnalyzer::PhaseEvent::NONE) {
        this->handlePhaseEvent(phase_event);
      }
    }

    if (!_recording_layer && _record_interface.active()) {
      _recording_layer = this->newRecording();
      _write_antipop_filter.trigger();
    } else if (_recording_layer && !_record_interface.active()) {
      this->endRecording();
    }

    if (_recording_layer) {
      float in =  _write_antipop_filter.process(_record_interface.in);
      _recording_layer->write(_timeline_position, in, _record_interface.strength, phase_defined);
    }
  }

};

} // namespace gko
} // namespace dsp
} // namepsace myrisa
