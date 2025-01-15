#pragma once

#include <vector>
#include <numeric> // std::iota

#include "rack.hpp"

#include "Song.hpp"
#include "Cycle.hpp"
#include "Observer.hpp"
#include "Conductor.hpp"
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
  std::vector<float> _next_cycles_song_love;
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

  static inline void adjustSizeToSize(std::vector<float> &vec, unsigned int size) {
    if (vec.size() < size) {
      while (vec.size() < size) {
        vec.push_back(0.f);
      }
    } else if (size < vec.size()) {
      vec.resize(size);
    }
  }

public:
  inline void updateSongCyclesLove(std::vector<Cycle*> &cycles, Group* new_cycle_group, int current_movement_i) {
    adjustSizeToSize(_next_cycles_song_love, cycles.size());
    adjustSizeToSize(_next_cycles_observer_love, cycles.size());

    if (_love_calculator_divider.process()) {
      for (unsigned int i = 0; i < cycles.size(); i++) {

        if (!cycles[i]->is_playing) {
          _next_cycles_observer_love[i] = 0.f;
          _next_cycles_song_love[i] = 0.f;
          continue;
        }

        // TODO
        // if (lock_observered_group)

        float cycle_i_observer_love = 0.f;
        float cycle_i_song_love = 1.f;

        if (Observer::checkIfCycleInGroupOneIsObservedByCycleInGroupTwo(cycles[i]->immediate_group, new_cycle_group)) {
          cycle_i_observer_love = 1.f;
        }

        for (unsigned int j = i + 1; j < cycles.size(); j++) {
          if (current_movement_i < cycles[j]->enter_at_movement_i) {
            continue;
          }

          if (Observer::checkIfCycleInGroupOneIsObservedByCycleInGroupTwo(cycles[i]->immediate_group, cycles[j]->immediate_group)) {
            cycle_i_song_love *= (1.f - cycles[j]->readLove());
            if (cycle_i_song_love <= 0.0001f) {
              cycle_i_song_love = 0.f;
              break;
            }
          }
        }

        _next_cycles_song_love[i] = cycle_i_song_love;
        _next_cycles_observer_love[i] = cycle_i_observer_love;
      }
    }

    for (unsigned int i = 0; i < cycles.size(); i++) {
      cycles[i]->song_love = smoothLoveValue(_next_cycles_song_love[i], cycles[i]->song_love);
      cycles[i]->observer_love = smoothLoveValue(_next_cycles_observer_love[i], cycles[i]->observer_love);
    }
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
