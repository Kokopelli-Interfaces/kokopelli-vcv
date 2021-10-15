#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;
using namespace kokopellivcv::dsp;

void Engine::setCircleToActiveLayer() {
  if (0 < _timeline.layers.size()) {
    _circle.first = _timeline.layers[_active_layer_i]->_start_beat;
    _circle.second =  _circle.first + _timeline.layers[_active_layer_i]->_n_beats;
    _timeline_position.beat = _circle.first;
  } else {
    _circle.first = 0;
    _circle.second = 1;
    _timeline_position.beat = 0;
  }

  _loop_length = _circle.second - _circle.first;

  _read_antipop_filter.trigger();
  _write_antipop_filter.trigger();
}
