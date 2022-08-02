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
  float love_resolution = 1000.f;

  rack::dsp::ClockDivider _love_calculator_divider;
  std::vector<float> _next_cycles_love;

  LoveUpdater() {
    _love_calculator_divider.setDivision(love_resolution);
  }

  inline void updateLoveResolution(float resolution) {
    love_resolution = resolution;
    _love_calculator_divider.reset();
    _love_calculator_divider.setDivision(resolution);
  }

private:
  // TODO cycle types (song or movement), depends on tuning and affects love
  inline float smoothValue(float current, float old) {
    float lambda = 1 / love_resolution;
    return old + (current - old) * lambda;
  }

public:
  inline void updateSongCyclesLove(std::vector<Cycle*> &cycles) {
    // TODO maybe put in function so to not call every step
    if (_next_cycles_love.size() < cycles.size()) {
      while (_next_cycles_love.size() < cycles.size()) {
        _next_cycles_love.push_back(0.f);
      }
    }

    if (_love_calculator_divider.process()) {
      for (unsigned int i = 0; i < cycles.size(); i++) {
        float cycle_i_love = 1.f;
        for (unsigned int j = i + 1; j < cycles.size(); j++) {
          if (Observer::checkIfCycleInGroupOneIsObservedByCycleInGroupTwo(cycles[i]->immediate_group, cycles[j]->immediate_group)) {
            cycle_i_love *= (1.f - cycles[j]->readLove());
            if (cycle_i_love <= 0.0001f) {
              cycle_i_love = 0.f;
              break;
            }
          }
        }

        _next_cycles_love[i] = cycle_i_love;
      }
    }

    for (unsigned int i = 0; i < cycles.size(); i++) {
      cycles[i]->love = smoothValue(_next_cycles_love[i], cycles[i]->love);
    }
  }
};

} // namespace hearth
} // namespace dsp
} // namespace kokopellivcv
