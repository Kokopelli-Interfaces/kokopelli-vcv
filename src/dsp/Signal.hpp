#pragma once

#include <rack.hpp>
#include "Saturate.hpp"

namespace myrisa {
namespace dsp {

enum SignalType { AUDIO, PARAM, CV, GATE, VOCT, VEL };

inline float attenuate(float sample, float attenuation, SignalType signal_type) {
  assert(0.f <= attenuation);
  assert(attenuation <= 1.0f);

  switch (signal_type) {
  case SignalType::GATE:
    return attenuation == 1.0f ? 0.f : sample;
  case SignalType::VOCT:
    return sample;
  default:
    return sample * (1.0f - attenuation);
  }
}

inline float sum(float sample_1, float sample_2, SignalType signal_type) {
  switch (signal_type) {
  case SignalType::AUDIO:
    return myrisa::dsp::saturate(sample_1 + sample_2);
  case SignalType::PARAM: case SignalType::VEL:
    return rack::clamp(sample_1 + sample_2, 0.f, 10.f);
  case SignalType::CV:
    return rack::clamp(sample_1 + sample_2, -10.f, 10.f);
  case SignalType::GATE:
    return (sample_1 == 10.f || sample_2 == 10.f) ? 10.f : 0.f;
  case SignalType::VOCT:
    return sample_2;
  default:
    return sample_1 + sample_2;
  }
}

inline float crossfade(float sample_1, float sample_2, float fade, SignalType signal_type) {
  switch (signal_type) {
  case SignalType::VEL:
  case SignalType::VOCT:
  case SignalType::GATE:
    return sample_1;
  default:
    return rack::crossfade(sample_1, sample_2, fade);
  }
}

} // namespace dsp
} // namespace myrisa
