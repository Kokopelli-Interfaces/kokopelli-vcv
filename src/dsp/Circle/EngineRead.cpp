#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;

float Engine::readAll() {
  float timeline_out = this->song.read();

  // FIXME assumes all selected
  timeline_out = timeline_out * (1 - this->inputs.love);

  if (this->options.use_antipop) {
    timeline_out = _read_antipop_filter.process(timeline_out);
  }

  return kokopellivcv::dsp::sum(timeline_out, this->inputs.in, _signal_type);
}

// FIXME, only established
float Engine::readEstablished() {
  float timeline_out = this->song.read();
  return timeline_out;
}
