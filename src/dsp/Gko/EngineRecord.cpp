#include "Engine.hpp"

using namespace myrisa::dsp::gko;

void Engine::record() {
  assert(_record.active);
  assert(_recording != nullptr);

  bool phase_defined = (_use_ext_phase || _phase_oscillator.isSet());
  if (!phase_defined) {
    _recording->pushBack(_record.in, _record.strength);
    _recording->samples_per_beat++;
  } else if (_recording->writableAtTime(_time)) {
    _recording->write(_time, _record.in, _record.strength);
  }
}
