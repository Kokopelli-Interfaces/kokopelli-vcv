#pragma once

#include "CircleGroupVoiceAttenuationCalculator.hpp"
#include "definitions.hpp"
#include "util/math.hpp"
#include <vector>

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct CircleGroup {
  // TODO recorded by gko
  float _love = 0.f;
  TimePosition _play_head;
  TimePosition _last_outer_play_head;

  std::vector<CircleVoice*> _voices;
  CircleGroupVoiceAttenuationCalculator _attenuation_calculator;

  // TODO use PhaseDivider
  inline TimePosition advancePlayHead(TimePosition outer_play_head) {
    _play_head.phase = outer_play_head.phase;
    if (_last_outer_play_head.beat != _outer_play_head.beat) {
      unsigned int n_group_beats = this->getPeriod();
      if (n_group_beats != 0) {
        _play_head.beat = _last_outer_play_head.beat % n_group_beats;
      } else {
        _play_head.beat = 0;
      }
    }
    _last_outer_play_head = outer_play_head;
  }

  // TODO account for New Love as well
  inline float step(TimePosition outer_play_head, RecordParams params) {
    advancePlayHead(outer_play_head);

    float group_out = 0.f;
    for (unsigned int i = 0; i < _voices.size(); i++) {
      float voice_out = _voices[i].step(_play_head, params);
      float voice_attenuation = _attenuation_calculator.getAttenuation(i);
      voice_out = kokopellivcv::dsp::attenuate(voice_out, voice_attenuation, signal_type);
      // TODO may be large on performance, test without
      group_out = kokopellivcv::dsp::sum(group_out, voice_out, signal_type);
    }

    _attenuation_calculator.step(_play_head, _voices);

    return group_out;
  }

  inline float readLove(TimePosition outer_play_head) {
    // TODO read END love of _voice 0 if past buffer so for easier transitions
    return _love;
  }

  inline unsigned int getPeriod() {
    unsigned int max_n_beats = 0;
    for (auto voice: _voices) {
      if (voice->isLooping() && max_n_beats < voice->_n_beats) {
        max_n_beats = voice->_n_beats;
      }
    }

    return max_n_beats;
  }

  inline void assertInvariant() {
    // assert group p = max(group voices p)
    // assert group p % (any group voice) = 0
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
