#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;

void Engine::nextMember() {
  if (this->isRecording()) {
    this->endRecording(false);

    // TODO put member in NEW circle
    _circle.first = _timeline_position.beat;
    _circle.second = _circle.first + 1;
    _loop_length = 1;
  } else {
    // TODO seek previous layer from current beat, may not be active
    if (_active_layer_i == _timeline.layers.size()-1) {
      _active_layer_i = 0;
    } else {
      _active_layer_i++;
    }

    skipToActiveLayer();
  }
}

void Engine::prevMember() {
  if (this->isRecording()) {
    // TODO create sub-circle
  } else {
    // TODO prev member
    if (_active_layer_i == 0) {
      _active_layer_i = _timeline.layers.size()-1;
    } else {
      _active_layer_i--;
    }

    skipToActiveLayer();
  }
}
