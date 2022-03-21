#include "Engine.hpp"

using namespace kokopellivcv::dsp::hearth;
using namespace kokopellivcv::dsp;

void Engine::toggleMovementProgression() {
  _gko.conductor.loop_movement = !_gko.conductor.loop_movement;
}

void Engine::step() {
  _gko.advance(_village, inputs, options);
}

int Engine::getMostRecentVoiceLength() {
  Voice* recent_voice = _village.voices[_village.voices.size()-1];
  return recent_voice->immediate_group->convertToBeat(recent_voice->period, false);
}

void Engine::cycleBackward() {
  printf("Cycling backward\n");
  _gko.cycleBackward(_village);
}

void Engine::cycleForward() {
  _gko.cycleForward(_village);
}

void Engine::voiceObservation() {
  _gko.voiceObservation(_village);
}

void Engine::ascend() {
  _gko.observer.ascend(_village);
  _gko.nextVoice(_village, VoiceEnd::DISCARD);
  _gko._discard_voice_at_next_love_return = true;
}

void Engine::undo() {
  _gko.undoVoice(_village);
}

void Engine::channelStateReset() {
  _gko.nextVoice(_village, VoiceEnd::DISCARD);
  _gko.love_updater._love_calculator_divider.reset();
}
