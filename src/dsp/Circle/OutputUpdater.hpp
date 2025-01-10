#pragma once

#include <vector>
#include <numeric> // std::iota

#include "rack.hpp"

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

class OutputUpdater {
public:

  inline void updateOutput(Outputs &out, std::vector<Cycle*> cycles, Group* new_cycle_group, float signal_in, float love_in, Options options) {
    // FIXME get rid of me
    kokopellivcv::dsp::SignalType signal_type = kokopellivcv::dsp::SignalType::AUDIO;

    out.sun = 0.f;
    out.observed_sun = 0.f;

    for (unsigned int i = 0; i < cycles.size(); i++) {
      float cycle_out = cycles[i]->readSignal(options.fade_time_mult);
      float cycle_out_observer = cycle_out * cycles[i]->observer_love;
      out.observed_sun = kokopellivcv::dsp::sum(out.observed_sun, cycle_out_observer, signal_type);

      if (Observer::checkIfCycleInGroupOneIsObservedByCycleInGroupTwo(cycles[i]->immediate_group, new_cycle_group)) {
        cycle_out *= (1.f - love_in);
      }

      out.sun = kokopellivcv::dsp::sum(out.sun, cycle_out, signal_type);
    }

    out.attenuated_observed_sun = out.observed_sun * (1.f - love_in);
    out.sun = kokopellivcv::dsp::sum(out.sun, signal_in, signal_type);
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
