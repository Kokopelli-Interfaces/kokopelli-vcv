#include "Engine.hpp"

using namespace myrisa::dsp::gko;

// TODO just read rendered timeline
float Engine::read() {
  float out = _timeline.read(_time);
  if (_record.active) {
    for (int i : _record.selected_layers) {
      if (_timeline.layers[i]->readableAtTime(_time)) {
        out -= _timeline.layers[i]->readSignal(_time);
        out += _timeline.layers[i]->readSignal(_time) * (1.f - _record.strength);
      }
    }
  }

  return out;
}
