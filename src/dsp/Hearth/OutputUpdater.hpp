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
namespace circle {

class OutputUpdater {
public:
  inline void updateOutput(Outputs &out, std::vector<Cycle*> cycles, Group* new_cycle_group, Inputs inputs, Options options) {
    // FIXME get rid of me
    kokopellivcv::dsp::SignalType signal_type = kokopellivcv::dsp::SignalType::AUDIO;

    out.sun = 0.f;
    out.observed_sun = 0.f;

    for (unsigned int i = 0; i < cycles.size(); i++) {
      float cycle_out = cycles[i]->readSignal();
      if (Observer::checkIfCycleInGroupOneIsObservedByCycleInGroupTwo(cycles[i]->immediate_group, new_cycle_group)) {
        out.observed_sun = kokopellivcv::dsp::sum(out.observed_sun, cycle_out, signal_type);
        cycle_out *= (1.f - inputs.love);
      }

      out.sun = kokopellivcv::dsp::sum(out.sun, cycle_out, signal_type);
    }

    out.attenuated_observed_sun = out.observed_sun * (1.f - inputs.love);

    if (options.include_moon_in_sun_output) {
      float add = inputs.in;
      if (!options.include_unloved_moon_in_sun_output && inputs.love < 0.01) {
        add *= 100 * inputs.love;
      }

      out.sun = kokopellivcv::dsp::sum(out.sun, add, signal_type);
    }
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
