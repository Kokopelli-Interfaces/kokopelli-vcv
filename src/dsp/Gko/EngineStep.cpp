#include "Engine.hpp"

using namespace myrisa::dsp::gko;
using namespace myrisa::dsp;

inline bool Engine::phaseDefined() {
  return _use_ext_phase || _phase_oscillator.isSet();
}

inline void Engine::handlePhaseEvent(PhaseAnalyzer::PhaseEvent event) {
  assert(phaseDefined());

  bool phase_flip = (event == PhaseAnalyzer::PhaseEvent::FORWARD || event == PhaseAnalyzer::PhaseEvent::BACKWARD);
  if (_recording_layer && phase_flip) {
    assert(_recording_layer != nullptr);
    assert(_recording_layer->_in->_samples_per_beat != 0);

    bool reached_recording_end = _recording_layer->_start_beat + _recording_layer->_n_beats <= _timeline_position.beat;
    if (reached_recording_end) {
      if (_record_interface.mode == RecordInterface::Mode::DUB) {
      printf("DUB END\n");
        endRecording();
        _recording_layer = newRecording();
      } else if (_record_interface.mode == RecordInterface::Mode::EXTEND) {
        _recording_layer->_n_beats++;
        printf("extend recording to: %d\n", _recording_layer->_n_beats);
      }
    }
  }

  if (event == PhaseAnalyzer::PhaseEvent::DISCONTINUITY && _options.use_antipop) {
    _read_antipop_filter.trigger();
  }
}

inline PhaseAnalyzer::PhaseEvent Engine::advanceTimelinePosition() {
  float internal_phase = _phase_oscillator.step(_sample_time);
  _timeline_position.phase = _use_ext_phase ? _ext_phase : internal_phase;

  PhaseAnalyzer::PhaseEvent phase_event = _phase_analyzer.process(_timeline_position.phase, _sample_time);
  if (phase_event == PhaseAnalyzer::PhaseEvent::BACKWARD && 1 <= _timeline_position.beat) {
    _timeline_position.beat--;
  } else if (phase_event == PhaseAnalyzer::PhaseEvent::FORWARD) {
    _timeline_position.beat++;
  }

  return phase_event;
}

void Engine::step() {
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
