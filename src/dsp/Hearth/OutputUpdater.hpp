#pragma once

#include <vector>
#include <numeric> // std::iota

#include "rack.hpp"

#include "definitions.hpp"
#include "Voice.hpp"
#include "Observer.hpp"
#include "Group.hpp"
#include "util/math.hpp"
#include "dsp/Signal.hpp"

namespace kokopellivcv {
namespace dsp {
namespace hearth {

class OutputUpdater {
public:
  inline void updateOutput(Outputs &out, std::vector<Voice*> voices, Group* new_voice_group, Inputs inputs, Options options) {
    // FIXME get rid of me
    kokopellivcv::dsp::SignalType signal_type = kokopellivcv::dsp::SignalType::AUDIO;

    out.sun = 0.f;
    out.observed_group = 0.f;

    for (unsigned int i = 0; i < voices.size(); i++) {
      float voice_out = voices[i]->readSignal();
      if (Observer::checkIfVoiceInGroupOneIsObservedByVoiceInGroupTwo(voices[i]->immediate_group, new_voice_group)) {
        out.observed_group = kokopellivcv::dsp::sum(out.observed_group, voice_out, signal_type);
        voice_out *= (1.f - inputs.love);
      }

      out.sun = kokopellivcv::dsp::sum(out.sun, voice_out, signal_type);
    }

    out.attenuated_observed_group = out.observed_group * (1.f - inputs.love);

    if (options.include_moon_in_sun_output) {
      float add = inputs.in;
      if (!options.include_unloved_moon_in_sun_output && inputs.love < 0.01) {
        add *= 100 * inputs.love;
      }

      out.sun = kokopellivcv::dsp::sum(out.sun, add, signal_type);
    }
  }
};

} // namespace hearth
} // namespace dsp
} // namespace kokopellivcv
