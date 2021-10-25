#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;

void Engine::forget() {
  if (isRecording()) {
    endRecording(false, false);
  }

  // if this was an unfixed recording, just leave it to make transitions easier
  // in fix_bounds mode, it acts as 'restart' or 'clear' memory
  if (!isRecording() || _record_params.fix_bounds) {
    int last_i = _timeline.layers.size()-1;
    if (0 <= last_i) {
      _circle = _timeline.layers[last_i]->_circle_before;
      unsigned int circle_length = _circle.second - _circle.first;

      while (_circle.second <= _timeline_position.beat) {
        _timeline_position.beat -= circle_length;
      }

      this->deleteLayer(last_i);
    }
  }
}

void Engine::loop() {
  if (this->isRecording()) {
    if (_record_params.fix_bounds) {
      this->endRecording(true, false);
    } else {
      this->endRecording(true, true);
      _record_params.fix_bounds = true;
    }
  } else {
    skipToActiveLayer();
  }
}

void Engine::toggleLayerMode() {
  unsigned int layer_i = _active_layer_i;

  if (isRecording()) {
    loop();
    layer_i = _timeline.layers.size()-1;
  }

  if (!_layer_mode) {
    _layer_mode = true;
    _selected_layers_idx_before_layer_mode = _selected_layers_idx;
    _circle_before_layer_mode = _circle;

    setCircleToLayer(layer_i);
    _timeline_position.beat = _circle.first;
    _read_antipop_filter.trigger();
    soloSelectLayer(_active_layer_i);
  } else {
    _selected_layers_idx = _selected_layers_idx_before_layer_mode;
    _circle = _circle_before_layer_mode;
    _layer_mode = false;
  }
}
