#pragma once

#include "definitions.hpp"
#include "util/math.hpp"
#include <vector>

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct CircleGroupVoiceAttenuationCalculator {
  std::vector<float> _current_attenuation;
  std::vector<float> _last_calculated_attenuation;
  rack::dsp::ClockDivider _attenuation_calculator_divider;

  CircleGroupVoiceAttenuationCalculator() {
    _attenuation_calculator_divider.setDivision(2000);
  }

  static inline float smoothValue(float current, float old) {
    const float lambda = 30.f / 44100;
    return old + (current - old) * lambda;
  }

  inline float getAttenuation(int voice_id) {
    return _current_attenuation[voice_id]
  }

  inline void step(TimePosition position, std::vector<CircleVoice*> voices) {
    // FIXME increase vector size if new voices
    if (_attenuation_calculator_divider.process()) {
      for (unsigned int voice_i = 0; voice_i < voices.size(); voice_i++) {
        float voice_i_attenuation = 0.f;
        for (unsigned int j = voice_i + 1; j < voices.size(); j++) {
          voice_i_attenuation += voices[j]->readLove(position);

          if (1.f <= voice_i_attenuation)  {
            voice_i_attenuation = 1.f;
            break;
          }
        }

        _last_calculated_attenuation[voice_i] = voice_i_attenuation;
      }
    }

    for (unsigned int i = 0; i < voices.size(); i++) {
      _current_attenuation[i] = smoothValue(_last_calculated_attenuation[i], _current_attenuation[i]);
    }
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
