#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;

float Engine::readSun() {
  return _song.out.sun;
}

// FIXME, only established
float Engine::readEstablished() {
  return _song.out.established;
}
