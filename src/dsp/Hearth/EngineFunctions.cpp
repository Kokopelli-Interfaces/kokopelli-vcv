#include "Engine.hpp"

using namespace kokopellivcv::dsp::hearth;
using namespace kokopellivcv::dsp;

void Engine::toggleMovementProgression() {
  _gko.toggleMovementProgression();
}

void Engine::step() {
  _gko.advance(&_village, this->inputs, this->options);
}

int Engine::getMostRecentVoiceLength() {
  Voice* recent_voice = _village.voices[_village.voices.size()-1];
  return recent_voice->immediate_group->convertToBeat(recent_voice->period, false);
}

void Engine::cycleBackward() {
  _gko.cycleBackward(&_village);
}

void Engine::cycleForward() {
  _gko.cycleForward(&_village);
}

void Engine::cycleObservation() {
  _gko.cycleObservation(&_village);
}

void Engine::ascend() {
  _gko.ascend(&_village);
}

void Engine::undo() {
  _gko.undoVoice(&_village);
}

void Engine::channelStateReset() {
  _gko.resetState(&_village);
}
