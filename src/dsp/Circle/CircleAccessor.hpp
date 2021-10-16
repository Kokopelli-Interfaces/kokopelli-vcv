#pragma once

#include "definitions.hpp"
#include "CircleGroup.hpp"
#include "CircleMember.hpp"
#include "dsp/PhaseDivider.hpp"

namespace kokopellivcv {
namespace dsp {
namespace circle {

typedef std::vector<unsigned int> circle_voice_id;
typedef std::variant<CircleMember*, CircleGroup*> CircleVoice;

struct CircleAccessor {
  static inline CircleVoice getVoice(CircleGroup* base_group, circle_voice_id id) {

    CircleVoice voice;
    for (unsigned int voice_i: id) {

      voice = base_group[voice_i];

      auto str = std::get<std::string>(var);
      auto* str  = std::get_if<std::string>(&var);
    }

    return voice;
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
