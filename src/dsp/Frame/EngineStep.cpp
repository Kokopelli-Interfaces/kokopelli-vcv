#include "Engine.hpp"

using namespace myrisa::dsp::frame;

inline void Engine::manifest() {
  assert(_active_manifestation.content != nullptr);

  if (_timeline.n_beats == 0) {
    Section* first_section = new Section();
  }

  // boundary conditions
  if (_timeline.n_beats <= _time.beat) {

  }


  if (!phase_defined) {
    _active_manifestation.content->buffer->pushBack(_manifest.in);
    _active_manifestation.content->manifestation_strength->pushBack(_manifest.strength);
  }

  // Section* new_section = new Section();
  // new_section.n_beats = 1;
  // _timeline.sections.push_back(new Section());
  // _timeline.n_beats

}

void Engine::step() {
  bool phase_defined = (_use_ext_phase || _phase_oscillator.isSet());
  if (phase_defined) {
    advancePosition();
  }

  if (_manifest.active) {
    manifest();
  }
}
