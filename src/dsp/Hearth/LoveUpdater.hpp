#pragma once

#include <vector>
#include <numeric> // std::iota

#include "rack.hpp"

// FIXME
#include "Village.hpp"
#include "Voice.hpp"
#include "Observer.hpp"
#include "Group.hpp"
#include "definitions.hpp"
#include "SignalCapture.hpp"
#include "Movement.hpp"
#include "TimeAdvancer.hpp"
#include "util/math.hpp"
#include "dsp/Signal.hpp"

namespace kokopellivcv {
namespace dsp {
namespace hearth {

class LoveUpdater {
public:
  float _love_resolution = 1000.f;

  rack::dsp::ClockDivider _love_calculator_divider;
  std::vector<float> _next_voices_love;

  LoveUpdater() {
    _love_calculator_divider.setDivision(_love_resolution);
  }

  inline void updateLoveResolution(float resolution) {
    _love_resolution = resolution;
    _love_calculator_divider.reset();
    _love_calculator_divider.setDivision(resolution);
  }

  inline void resetState() {
    _love_calculator_divider.reset();
  }

private:
  // TODO voice types (song or movement), depends on tuning and affects love
  inline float smoothValue(float current, float old) {
    float lambda = 1 / _love_resolution;
    return old + (current - old) * lambda;
  }

public:
  inline void updateVillageVoicesLove(std::vector<Voice*> &voices) {
    // TODO maybe put in function so to not call every step
    if (_next_voices_love.size() < voices.size()) {
      while (_next_voices_love.size() < voices.size()) {
        _next_voices_love.push_back(0.f);
      }
    }

    if (_love_calculator_divider.process()) {
      for (unsigned int i = 0; i < voices.size(); i++) {
        float voice_i_love = 1.f;
        for (unsigned int j = i + 1; j < voices.size(); j++) {
          if (Observer::checkIfVoiceInGroupOneIsObservedByVoiceInGroupTwo(voices[i]->immediate_group, voices[j]->immediate_group)) {
            voice_i_love *= (1.f - voices[j]->readLove());
            if (voice_i_love <= 0.0001f) {
              voice_i_love = 0.f;
              break;
            }
          }
        }

        _next_voices_love[i] = voice_i_love;
      }
    }

    for (unsigned int i = 0; i < voices.size(); i++) {
      voices[i]->love = smoothValue(_next_voices_love[i], voices[i]->love);
    }
  }
};

} // namespace hearth
} // namespace dsp
} // namespace kokopellivcv
