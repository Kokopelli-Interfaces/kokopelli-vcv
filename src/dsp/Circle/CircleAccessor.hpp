#pragma once

#include "definitions.hpp"
#include "CircleGroup.hpp"
#include "CircleMember.hpp"
#include "dsp/PhaseDivider.hpp"

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct CircleAccessor {
  static inline CircleVoice getVoice(CircleGroup* base_group, voice_id id) {
    CircleGroup* next_group = base_group;
    for (unsigned int i = 0; i < id.size() - 1; i++) {
      unsigned int voice_i = id[i];
      next_group = std::get<CircleGroup*>(next_group->_voices[voice_i])
    }

    unsigned int voice_i = id[id.size()-1];
    return next_group->_voices[voice_i];
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
