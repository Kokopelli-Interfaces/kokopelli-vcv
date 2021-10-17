#pragma once

#include "definitions.hpp"
#include "CircleGroup.hpp"
#include "CircleMember.hpp"
#include "dsp/PhaseDivider.hpp"

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct CircleAccessor {
  static inline CircleVoice getVoice(CircleGroup* base_group, circle_voice_id voice_id) {
    CircleGroup* next_group = base_group;
    for (unsigned int i = 0; i < voice_id.size() - 1; i++) {
      unsigned int voice_i = voice_id[i];
      next_group = next_group->_voices[voice_i].group;
    }

    unsigned int voice_i = voice_id[voice_id.size()-1];
    return next_group->_voices[voice_i];
  }

  static inline CircleMember* getMember(CircleGroup* base_group, circle_voice_id voice_id) {
    return getVoice(base_group, voice_id).member;
  }

  static inline CircleGroup* getGroup(CircleGroup* base_group, circle_voice_id voice_id) {
    return getVoice(base_group, voice_id).group;
  }

  static inline CircleGroup* getVoicesGroup(CircleGroup* base_group, circle_voice_id id_of_voice_in_group) {
    if (id_of_voice_in_group.size() == 1) {
      return base_group;
    } else {
      circle_voice_id group_circle_voice_id = id_of_voice_in_group;
      group_circle_voice_id.pop_back();
      return getVoice(base_group, group_circle_voice_id).group;
    }
  }

  static inline circle_voice_id putVoiceIntoBaseGroup(CircleGroup* base_group, CircleVoice voice) {
    base_group->addVoice(voice);
    circle_voice_id new_circle_voice_id;
    new_circle_voice_id.push_back(base_group->getNumberOfVoices()-1);
    return new_circle_voice_id;
  }

  static inline circle_voice_id putVoiceIntoSubGroup(CircleGroup* base_group, CircleVoice voice, circle_voice_id group_id) {
    CircleGroup* group = getVoice(base_group, group_id).group;
    group->addVoice(voice);

    circle_voice_id new_voice_id = group_id;
    unsigned int voice_i_in_group = group->getNumberOfVoices() - 1;
    new_voice_id.push_back(voice_i_in_group);
    return new_voice_id;
  }

  static inline circle_voice_id putVoiceIntoVoicesGroup(CircleGroup* base_group, CircleVoice voice, circle_voice_id voice_in_group) {
    CircleGroup* group = getVoicesGroup(base_group, voice_in_group);
    group->addVoice(voice);
    circle_voice_id new_voice_id = voice_in_group;
    new_voice_id.back() = group->getNumberOfVoices()-1;
    return new_voice_id;
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
