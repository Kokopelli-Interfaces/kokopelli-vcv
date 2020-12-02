#include "Engine.hpp"

using namespace myrisa::dsp::gko;

void Engine::record() {
  assert(_record_params.active);
  assert(_recording != nullptr);

  bool phase_defined = (_use_ext_phase || _phase_oscillator.isSet());
  if (!phase_defined) {
    _recording->pushBack(_record_params.in, _record_params.strength);
    _recording->samples_per_beat++;
  } else if (_recording->writableAtPosition(_timeline_position)) {
    _recording->write(_timeline_position, _record_params.in, _record_params.strength);
  } else {
    printf("Not writable \n");
  }
}
