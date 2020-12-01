#include "Engine.hpp"

using namespace myrisa::dsp::gko;

void Engine::manifest() {
  assert(_manifestation != nullptr);
  assert(_manifest.active);

  bool phase_defined = (_use_ext_phase || _phase_oscillator.isSet());
  if (!phase_defined) {
    _manifestation->pushBack(_manifest.in, _manifest.strength);
  } else {
    assert(0 < _manifestation->signal->size());
    assert(0 < _manifestation->manifestation_strength->size());

    _manifestation->write(_time, _manifest.in, _manifest.strength);
  }
}
