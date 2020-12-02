#include "Engine.hpp"

using namespace myrisa::dsp::gko;

// TODO just read rendered timeline
float Engine::read() {
  float out = _timeline.read(_timeline_position);
  if (_record_params.active) {
    for (int i : _record_params.selected_layers) {
      if (_timeline.layers[i]->readableAtPosition(_timeline_position)) {
        out -= _timeline.layers[i]->readSignal(_timeline_position);
        out += _timeline.layers[i]->readSignal(_timeline_position) * (1.f - _record_params.strength);
      }
    }
  }

  return out;
}
