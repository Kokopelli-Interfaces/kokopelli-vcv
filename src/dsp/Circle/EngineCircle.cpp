#include "Engine.hpp"

using namespace tribalinterfaces::dsp::circle;
using namespace tribalinterfaces::dsp;

void Engine::setCircleToLayer(unsigned int layer_i) {
  _circle.first = _timeline.layers[layer_i]->_start_beat;
  _circle.second =  _circle.first + _timeline.layers[layer_i]->_n_beats;
}

void Engine::skipToActiveLayer() {
  if (0 < _timeline.layers.size()) {
    setCircleToLayer(_active_layer_i);
    _timeline_position.beat = _circle.first;
  } else {
    _circle.first = 0;
    _circle.second = 1;
    _timeline_position.beat = 0;
  }

  _read_antipop_filter.trigger();

  if (isRecording()) {
    delete _recording_layer;
    _recording_layer = nullptr;
    _write_antipop_filter.trigger();
  }
}
