#pragma once

#include "CircleGroupVoiceAttenuationCalculator.hpp"
#include "definitions.hpp"
#include "dsp/Signal.hpp"
#include "util/math.hpp"
#include <vector>

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct CircleGroup {
  // TODO recorded by gko
  float _love = 0.f;

  std::vector<CircleVoice*> _voices;
  CircleGroupVoiceAttenuationCalculator _attenuation_calculator;

  inline unsigned int getNumberOfVoices() {
    return _voices.size();
  }

  inline void addVoice(CircleVoice voice) {
    _voices->push_back(voice);
  }

  // TODO use PhaseDivider
  // inline TimePosition getPlayHead(TimePosition play_head) {
  //   _play_head.phase = play_head.phase;
  //   if (_last_play_head.beat != play_head.beat) {
  //     unsigned int n_group_beats = this->getPeriod();
  //     if (n_group_beats != 0) {
  //       _play_head.beat = _last_play_head.beat % n_group_beats;
  //     } else {
  //       _play_head.beat = 0;
  //     }
  //   }
  //   _last_play_head = play_head;
  // }

  // TODO account for New Love as well
  inline float step(TimePosition play_head, Inputs inputs, Parameters params) {
    float group_out = 0.f;
    for (unsigned int i = 0; i < _voices.size(); i++) {
      float voice_out = _voices[i].step(play_head, inputs, params);
      float voice_attenuation = _attenuation_calculator.getAttenuation(i);
      voice_out = kokopellivcv::dsp::attenuate(voice_out, voice_attenuation, params.signal_type);
      // TODO may be large on performance, test without
      group_out = kokopellivcv::dsp::sum(group_out, voice_out, params.signal_type);
    }

    _attenuation_calculator.step(play_head, _voices);

    return group_out;
  }

  inline float readLove(TimePosition play_head) {
    // TODO read END love of _voice 0 if past buffer so for easier transitions
    return _love;
  }

  inline unsigned int getPeriod() {
    // assertInvariant();

    unsigned int max_n_beats = 0;
    for (auto voice: _voices) {
      if (voice->isActive() && max_n_beats < voice->getPeriod()) {
        max_n_beats = voice->getPeriod();
      }
    }

    return max_n_beats;
  }

  inline bool isActive() {
    for (auto voice: _voices) {
      if (voice->isActive()) {
        return true;
      }
    }
    return false;
  }

  inline void assertInvariant() {
    // clock ?
    // assert group p = max(group voices p)
    // assert group p % (any group voice) = 0
    assert(0 < _voices.size());
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
