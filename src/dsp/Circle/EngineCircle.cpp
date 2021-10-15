#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;
using namespace kokopellivcv::dsp;

// TODO REMOVE unnecessary circle first second updates if updateCirclePeriod

// unsigned int Engine::updateCirclePeriod() {
// // TODO depends on active layers which are repeating at current timeline beat
// }

void Engine::skipToActiveLayer() {
  if (_timeline.layers.size() == 0) {
    _circle.first = 0;
    _circle.second = 1;
    _loop_length = 1;
    _timeline_position.beat = 0;
    return;
  }

  // FIXME
  _circle.first = _timeline.layers[_active_layer_i]->_start.beat;
  _circle.second =  _circle.first + _timeline.layers[_active_layer_i]->_n_beats;
  _loop_length = _circle.second - _circle.first;

  _timeline_position.beat = _circle.first;
  _read_antipop_filter.trigger();
  _write_antipop_filter.trigger();

  // TODO
  // updateCirclePeriod
}
