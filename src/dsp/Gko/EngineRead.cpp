#include "Engine.hpp"

using namespace tribalinterfaces::dsp::gko;

float Engine::read() {
  float timeline_out = _timeline.read(_timeline_position, _recording_layer, _record_params, _active_layer_i);

  if (_options.use_antipop) {
    timeline_out = _read_antipop_filter.process(timeline_out);
  }

  return timeline_out;
}

float Engine::readSelection() {
  float timeline_out = _timeline.readRawLayers(_timeline_position, _selected_layers_idx);
  return timeline_out;
}

float Engine::readActiveLayer() {
  return _timeline.active_layer_out;
}
