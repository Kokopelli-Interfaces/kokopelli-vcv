#include "Engine.hpp"

using namespace kokopelli::dsp::circle;
using namespace kokopelli::dsp;

inline bool Engine::phaseDefined() {
  return interface->_use_ext_phase || _phase_oscillator.isSet();
}

inline void Engine::setPhaseOscillator() {
  if (!_phase_oscillator.isSet()) {
    if (interface->use_ext_phase && _phase_analyzer.getDivisionPeriod() != 0) {
      _phase_oscillator.setFrequency(1 / _phase_analyzer.getDivisionPeriod());
    } else {
      float recording_time = _recording_member->_in->_samples_per_beat * interface->sample_time;
      _phase_oscillator.setFrequency(1 / recording_time);
    }
    printf("-- phase oscillator set with frequency: %f, sample time is: %f\n", _phase_oscillator.getFrequency(), interface->sample_time);
  }
}

inline int Engine::getSamplesPerBeat() {
  int samples_per_beat = 0;
  if (phaseDefined()) {
    if (interface->_use_ext_phase) {
      samples_per_beat = _phase_analyzer.getSamplesPerBeat(interface->sample_time);
    } else if (_phase_oscillator.isSet()) {
      float beat_period = 1 / _phase_oscillator.getFrequency();
      samples_per_beat = floor(beat_period / interface->sample_time);
    }
  }
  return samples_per_beat;
}
