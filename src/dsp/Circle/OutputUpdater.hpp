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

class OutputUpdater {
public:
  inline void updateOutput(Outputs &out, std::vector<Cycle*> cycles, Group* new_cycle_group, Inputs inputs) {
    // FIXME get rid of me
    kokopellivcv::dsp::SignalType signal_type = kokopellivcv::dsp::SignalType::AUDIO;

    out.sun = 0.f;
    out.established = 0.f;

    for (unsigned int i = 0; i < cycles.size(); i++) {
      float cycle_out = cycles[i]->readSignal();
      if (Observer::checkIfCycleInGroupOneIsObservedByCycleInGroupTwo(cycles[i]->immediate_group, new_cycle_group)) {
        out.established = kokopellivcv::dsp::sum(out.established, cycle_out, signal_type);
        cycle_out *= (1.f - inputs.love);
      }

      out.sun = kokopellivcv::dsp::sum(out.sun, cycle_out, signal_type);
    }

    out.attenuated_established = out.established * (1.f - inputs.love);
    out.sun = kokopellivcv::dsp::sum(out.sun, inputs.in, signal_type);
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv