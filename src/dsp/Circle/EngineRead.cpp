#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;

float Engine::readAll() {
  float song_out = this->_song.read();

  // FIXME assumes all selected
  song_out = song_out * (1 - this->inputs.love);

  // if (this->options.use_antipop) {
  //   song_out = _read_antipop_filter.process(song_out);
  // }

  return kokopellivcv::dsp::sum(song_out, this->inputs.in, _signal_type);
}

// FIXME, only established
float Engine::readEstablished() {
  float song_out = this->_song.read();
  return song_out;
}
