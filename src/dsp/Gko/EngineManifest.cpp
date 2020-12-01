#include "Engine.hpp"

using namespace myrisa::dsp::gko;

void Engine::manifest() {
  assert(_manifest.active);
  assert(_manifestation != nullptr);

  bool phase_defined = (_use_ext_phase || _phase_oscillator.isSet());
  if (!phase_defined) {
    _manifestation->pushBack(_manifest.in, _manifest.strength);
    _manifestation->samples_per_beat++;
  } else if (_manifestation->writableAtTime(_time)) {
    _manifestation->write(_time, _manifest.in, _manifest.strength);
  }
}
