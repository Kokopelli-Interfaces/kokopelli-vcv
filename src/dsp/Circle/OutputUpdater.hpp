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
#include "dsp/AntipopFilter.hpp"

namespace kokopellivcv {
namespace dsp {
namespace circle {

class OutputUpdater {
public:
  AntipopFilter antipop_filter;

  inline void updateOutput(Outputs &out, std::vector<Cycle*> cycles, Group* new_cycle_group, float signal_in, float love_in, Options options) {
    // FIXME get rid of me
    kokopellivcv::dsp::SignalType signal_type = kokopellivcv::dsp::SignalType::AUDIO;

    out.sun = 0.f;
    out.observed_sun = 0.f;
a
    for (unsigned int i = 0; i < cycles.size(); i++) {
      float cycle_out = cycles[i]->readSignal(options.fade_time_mult);
      if (Observer::checkIfCycleInGroupOneIsObservedByCycleInGroupTwo(cycles[i]->immediate_group, new_cycle_group)) {
        out.observed_sun = kokopellivcv::dsp::sum(out.observed_sun, cycle_out, signal_type);
        cycle_out *= (1.f - love_in);
      }

      out.sun = kokopellivcv::dsp::sum(out.sun, cycle_out, signal_type);
    }

    out.attenuated_observed_sun = out.observed_sun * (1.f - love_in);

    out.sun = kokopellivcv::dsp::sum(out.sun, signal_in, signal_type);

    if (options.use_antipop_filter) {
      out.sun = antipop_filter.process(out.sun, true);
      out.attenuated_observed_sun = antipop_filter.process(out.attenuated_observed_sun, false);
    }
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
