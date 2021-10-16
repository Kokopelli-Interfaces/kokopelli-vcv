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

  std::vector<CircleVoice*> _voices;
  CircleGroupVoiceAttenuationCalculator _attenuation_calculator;

  inline CircleVoice* getVoice(unsigned int voice_i) {

  }

  // TODO account for New Love as well
  inline float step(TimePosition position, CircleVoice* recording, RecordParams record_params, unsigned int active_voice_i) {
    float group_out = 0.f;
    for (unsigned int i = 0; i < _voices.size(); i++) {
      float voice_out = _voices[i].step();
      float voice_attenuation = _attenuation_calculator.getAttenuation(i);
      voice_out = kokopellivcv::dsp::attenuate(voice_out, voice_attenuation, signal_type);
      // TODO may be large on performance, test without
      group_out = kokopellivcv::dsp::sum(group_out, voice_out, signal_type);
    }

    _attenuation_calculator.step(position, _voices);

    return group_out;
  }

  inline float readLove(TimePosition position) {
    // TODO read END love of _voice 0 if past buffer so for easier transitions
    return _love;
  }

  inline unsigned int getPeriod(TimePosition position) {
    unsigned int max_n_beats = 0;
    for (auto voice: _voices) {
      if (voice->readableAtPosition(position) && voice->isLooping() && max_n_beats < voice->_n_beats) {
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
