#include "Engine.hpp"
#include "dsp/PhaseAnalyzer.hpp"

using namespace myrisa::dsp::gko;
using namespace myrisa::dsp;

inline void Engine::handlePhaseEvent(PhaseAnalyzer::PhaseEvent phase_event) {
  bool phase_flip = (phase_event == PhaseAnalyzer::PhaseEvent::FORWARD || phase_event == PhaseAnalyzer::PhaseEvent::BACKWARD);

  // _recorder
  if (phase_flip && _recorder->isRecording() && _recorder->pastRecordingEnd()) {
    if (_interface.record_interface.mode == RecordInterface::Mode::DUB) {
      printf("DUB END\n");
      _recorder->endRecording(_time_position_advancer, _layer_manager);
      _recorder->newRecording(_timeline_position, _interface.record_interface, _time_position_advancer.getSamplesPerBeat());
    } else if (_interface.record_interface.mode == RecordInterface::Mode::EXTEND) {
      _recorder->growRecording(1);
      printf("extend recording to: %d\n", _recorder->_recording_layer->_n_beats);
    }
  }

  // _reader
  if (_interface.options.use_antipop && phase_event == PhaseAnalyzer::PhaseEvent::DISCONTINUITY) {
    _reader->triggerAntipop();
  }
}

void Engine::step() {
  PhaseAnalzer::PhaseEvent phase_event = _time_position_advancer.step(_timeline_position, _interface);
  if (phase_event != PhaseAnalyzer::PhaseEvent::NONE) {
    this->handlePhaseEvent(phase_event)
  }

  if (!_recorder->isRecording() && _interface.record_interface.active()) {
    _recorder->newRecording();
  } else if (recorder->isRecording() && !_interface.record_interface.active()) {
    _recorder->endRecording();
  }
}
