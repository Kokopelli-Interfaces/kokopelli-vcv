#pragma once

#include "Member.hpp"
#include "definitions.hpp"
#include "util/math.hpp"
#include <vector>

namespace kokpelliinterfaces {
namespace dsp {
namespace circle {

/**
   The Circle is the top level structure for content.
*/
struct Circle {
  std::vector<Member*> members;
  float active_member_out = 0.f;

  float _sample_time = 1.0f;

  /** read only */

  std::vector<float> _current_attenuation;
  std::vector<float> _last_calculated_attenuation;
  rack::dsp::ClockDivider _attenuation_calculator_divider;

  Circle() {
    _attenuation_calculator_divider.setDivision(2000);
  }

  static inline float smoothValue(float current, float old) {
    const float lambda = 30.f / 44100;
    return old + (current - old) * lambda;
  }

  inline unsigned int getMemberIndexForPosition(TimePosition position) {
    if (members.size() == 0) {
      return 0;
    }

    unsigned int member_i = members.size()-1;
    for (int i = members.size()-1; 0 <= i; i--) {
      if (members[i]->_start_beat <= position.beat) {
        break;
      }

      member_i = i;
    }

    return member_i;
  }

  inline void updateMemberAttenuations(TimePosition position) {
    if (_attenuation_calculator_divider.process()) {
      for (unsigned int member_i = 0; member_i < members.size(); member_i++) {
        float member_i_attenuation = 0.f;
        for (unsigned int j = member_i + 1; j < members.size(); j++) {
          for (auto target_member_i : members[j]->members_being_observed_idx) {
            if (target_member_i == member_i) {
              member_i_attenuation += members[j]->readRecordingLove(position);
              break;
            }
          }

          if (1.f <= member_i_attenuation)  {
            member_i_attenuation = 1.f;
            break;
          }
        }

        _last_calculated_attenuation[member_i] = member_i_attenuation;
      }
    }

    for (unsigned int i = 0; i < members.size(); i++) {
      _current_attenuation[i] = smoothValue(_last_calculated_attenuation[i], _current_attenuation[i]);
    }
  }

  inline float hear(TimePosition position, Member* recording, RecordParams record_params, unsigned int active_member_i) {
    updateMemberAttenuations(position);

    // FIXME multiple recordings in member, have loop and array of types
    kokpelliinterfaces::dsp::SignalType signal_type = kokpelliinterfaces::dsp::SignalType::AUDIO;
    if (0 < members.size()) {
      signal_type = members[0]->_in->_signal_type;
    }

    float signal_out = 0.f;
    for (unsigned int i = 0; i < members.size(); i++) {
      if (members[i]->readableAtPosition(position)) {
        float attenuation = _current_attenuation[i];

        if (record_params.active()) {
          for (unsigned int sel_i : recording->members_being_observed_idx) {
            if (sel_i == i) {
              attenuation += record_params.love;
             break;
            }
          }
        }

        attenuation = rack::clamp(attenuation, 0.f, 1.f);
        float member_out = members[i]->sing(position);
        member_out = kokpelliinterfaces::dsp::attenuate(member_out, attenuation, signal_type);
        if (i == active_member_i) {
          active_member_out = member_out;
        }

        signal_out = kokpelliinterfaces::dsp::sum(signal_out, member_out, signal_type);
      }
    }

    return signal_out;
  }

  inline std::vector<Member*> getMembersFromIdx(std::vector<unsigned int> member_idx) {
    std::vector<Member*> selected_members;
    for (auto member_id : member_idx) {
      selected_members.push_back(members[member_id]);
    }
    return selected_members;
  }

  // inline float getNumberOfBeatsOfMemberSelection(std::vector<unsigned int> member_idx) {
  //   assert(members.size() != 0);
  //   assert(member_idx.size() != 0);
  //   assert(member_idx.size() <= members.size());

  //   unsigned int max_n_beats = 0;

  //   for (auto member : getMembersFromIdx(member_idx)) {
  //     if (max_n_beats < member->_n_beats) {
  //       max_n_beats = member->_n_beats;
  //     }
  //   }

  //   return max_n_beats;
  // }

  // TODO all members have same beat. may be different though

  inline void newMember() {
  }

  inline void prevBeat() {
    for (auto member : members) {
      member->prevBeat();
    }
  }

  inline void nextBeat() {
    for (auto member : members) {
      member->nextBeat();
    }
  }

  inline listen() {
  }

  inline unsigned int getNumberOfBeats(TimePosition position) {
    unsigned int max_n_beats = 0;
    for (auto member: members) {
      if (member->readableAtPosition(position) && member->_loop && max_n_beats < member->_n_beats) {
        max_n_beats = member->_n_beats;
      }
    }

    return max_n_beats;
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokpelliinterfaces
