#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;

void Engine::nextMember() {
  if (this->isRecording()) {
    this->endRecording(false);
  } else {
    // TODO next member
  }
}

void Engine::prevMember() {
  if (this->isRecording()) {
    // TODO create sub-circle
  } else {
    // TODO prev member
  }
}
