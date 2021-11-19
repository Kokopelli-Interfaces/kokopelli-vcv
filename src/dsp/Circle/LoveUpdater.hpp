#pragma once

#include <vector>
#include <numeric> // std::iota

#include "rack.hpp"

// FIXME
#include "Song.hpp"
#include "Cycle.hpp"
#include "Observer.hpp"
#include "Group.hpp"
#include "definitions.hpp"
#include "TimeCapture.hpp"
#include "Movement.hpp"
#include "TimeAdvancer.hpp"
#include "util/math.hpp"
#include "dsp/Signal.hpp"

namespace kokopellivcv {
namespace dsp {
namespace circle {

class LoveUpdater {
public:
  float love_resolution = 1000.f;

  rack::dsp::ClockDivider _love_calculator_divider;
  std::vector<float> _next_cycles_love;

  LoveUpdater() {
    _love_calculator_divider.setDivision(5000);
  }

private:
  // TODO cycle types (song or movement), depends on tuning and affects love
  inline float smoothValue(float current, float old) {
    float lambda = this->love_resolution / 44100;
    return old + (current - old) * lambda;
  }

public:
  inline void updateSongCyclesLove(std::vector<Cycle*> &cycles) {
    if (_love_calculator_divider.process()) {
      for (unsigned int i = 0; i < cycles.size(); i++) {
        float cycle_i_love = 1.f;
        for (unsigned int j = i + 1; j < cycles.size(); j++) {
          if (Observer::checkIfCycleInGroupOneIsObservedByCycleInGroupTwo(cycles[i]->immediate_group, cycles[j]->immediate_group)) {
            cycle_i_love *= (1.f - cycles[j]->readLove());
            if (cycle_i_love <= 0.001f) {
              cycle_i_love = 0.f;
              break;
            }
          }
        }

        while (_next_cycles_love.size() <= i) {
          _next_cycles_love.push_back(0.f);
        }
        _next_cycles_love[i] = cycle_i_love;
      }
    }

    for (unsigned int i = 0; i < cycles.size(); i++) {
      cycles[i]->love = smoothValue(_next_cycles_love[i], cycles[i]->love);
    }
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
