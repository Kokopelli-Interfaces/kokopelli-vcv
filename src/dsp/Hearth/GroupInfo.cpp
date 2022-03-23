#include "Group.hpp"

using namespace kokopellivcv::dsp::hearth;
using namespace kokopellivcv::dsp;

void Group::undoLastVoiceWithoutUndoingParent() {
  assert(_voices_in_group.size() == _period_history.size());

  int last_i = _voices_in_group.size() - 1;
  _voices_in_group.pop_back();
  _period = _period_history[last_i];
  _period_history.pop_back();
  _next_voices_relative_love.pop_back();
}

void Group::undoLastVoice() {
  undoLastVoiceWithoutUndoingParent();

  if (this->parent_group) {
    this->parent_group->undoLastVoice();
  }
}

bool Group::checkIfVoiceIsInGroup(Voice* voice) {
  for (Voice* voice_in_group : _voices_in_group) {
    if (voice_in_group == voice) {
      return true;
    }
  }
  return false;
}

float Group::getBeatPhase() {
  if (_period != 0.0 && _beat_period != 0.0) {
    Time time_in_beat = std::fmod((float)this->playhead, (float)_beat_period);
    return rack::clamp(time_in_beat / _beat_period , 0.f, 1.f);
  }
  return 0.f;
}

float Group::getPhase() {
  if (_period != 0.0 && _beat_period != 0.0) {
    return rack::clamp(this->playhead / _period , 0.f, 1.f);
  }
  return 0.f;
}

int Group::getBeatN() {
  if (_period != 0.f && _beat_period != 0.f) {
    return (int) (this->playhead / _beat_period);
  }
  return 0;
}

int Group::convertToBeat(Time time, bool mod) {
  if (_period != 0.f && _beat_period != 0.f) {
    float time_in_observed_sun = time;
    if (mod && _period < time) {
      time_in_observed_sun = std::fmod((float)time, (float)_period);
    }

    return (int) (time_in_observed_sun / _beat_period);
  }
  return 0;
}

int Group::getTotalBeats() {
  if (_beat_period != 0.f) {
    return (_period / _beat_period);
  } else {
    return 0;
  }
}
