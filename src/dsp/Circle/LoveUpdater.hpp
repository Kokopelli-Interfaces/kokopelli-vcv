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
  std::vector<float> _next_cycles_observer_love;

  LoveUpdater() {
    _love_calculator_divider.setDivision(love_resolution);
  }

  inline void updateLoveResolution(float resolution) {
    love_resolution = resolution;
    _love_calculator_divider.reset();
    _love_calculator_divider.setDivision(resolution);
  }

private:
  inline float smoothLoveValue(float current, float old) {
    float lambda = 1 / love_resolution;
    return rack::clamp(old + (current - old) * lambda, 0.f, 1.f);
  }

public:
  inline void updateSongCyclesLove(std::vector<Cycle*> &cycles, Group* new_cycle_group) {
    if (_next_cycles_love.size() < cycles.size()) {
      while (_next_cycles_love.size() < cycles.size()) {
        _next_cycles_love.push_back(0.f);
      }
    }

    if (_next_cycles_observer_love.size() < cycles.size()) {
      while (_next_cycles_observer_love.size() < cycles.size()) {
        _next_cycles_observer_love.push_back(0.f);
      }
    }

    if (_love_calculator_divider.process()) {
      for (unsigned int i = 0; i < cycles.size(); i++) {
        if (Observer::checkIfCycleInGroupOneIsObservedByCycleInGroupTwo(cycles[i]->immediate_group, new_cycle_group)) {
          _next_cycles_observer_love[i] = 1.f;
        } else {
          _next_cycles_observer_love[i] = 0.f;
        }

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
      cycles[i]->love = smoothLoveValue(_next_cycles_love[i], cycles[i]->love);
      cycles[i]->observer_love = smoothLoveValue(_next_cycles_observer_love[i], cycles[i]->observer_love);
    }
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
