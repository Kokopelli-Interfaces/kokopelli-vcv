#pragma once

#include "Member.hpp"
#include "definitions.hpp"
#include "util/math.hpp"
#include <vector>

namespace kokopellivcv {
namespace dsp {
namespace circle {

/**
   The Timeline is the top level structure for content.
*/
struct Timeline {
  std::vector<Member*> members;

  /** read only */

  rack::dsp::ClockDivider _love_calculator_divider;
  float _love_resolution = 10000.f;
  std::vector<float> _next_love;
  unsigned int _n_loved_members = 0;

  Timeline() {
    _love_calculator_divider.setDivision(10000);
  }

  inline unsigned int getNumberOfActiveMembers() {
    return _n_loved_members;
  }

  inline float smoothValue(float current, float old) {
    const float lambda = _love_resolution / 44100;
    return old + (current - old) * lambda;
  }

  inline bool atEnd(TimePosition position) {
    if (members.size() == 0) {
      return true;
    }

    unsigned int last_member_i = members.size()-1;
    return members[last_member_i]->_start_beat + members[last_member_i]->_n_beats <= position.beat + 1;
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

  inline void updateMemberLove(TimePosition position) {
    if (_love_calculator_divider.process()) {
      unsigned int n_loved = 0;
      for (unsigned int member_i = 0; member_i < members.size(); member_i++) {

        float member_i_love = 1.f;
        for (unsigned int j = member_i + 1; j < members.size(); j++) {
          member_i_love -= members[j]->readLove(position);
          if (member_i_love <= 0.f)  {
            member_i_love = 0.f;
            break;
          }
        }

        if (0.f < member_i_love) {
          n_loved++;
        }

        _next_love[member_i] = member_i_love;
      }

      _n_loved_members = n_loved;
    }

    for (unsigned int i = 0; i < members.size(); i++) {
      members[i]->_love = smoothValue(_next_love[i], members[i]->_love);
    }
  }

  inline float read(TimePosition position) {
    updateMemberLove(position);

    // FIXME multiple recordings in member, have loop and array of types
    kokopellivcv::dsp::SignalType signal_type = kokopellivcv::dsp::SignalType::AUDIO;
    if (0 < members.size()) {
      signal_type = members[0]->_in->_signal_type;
    }

    float signal_out = 0.f;
    for (unsigned int i = 0; i < members.size(); i++) {
      if (members[i]->_love <= 0.f || !members[i]->readableAtPosition(position)) {
        continue;
      }

      float member_out = members[i]->readSignal(position) * members[i]->_love;

      signal_out = kokopellivcv::dsp::sum(signal_out, member_out, signal_type);
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

  inline float getNumberOfBeatsOfMemberSelection(std::vector<unsigned int> member_idx) {
    assert(members.size() != 0);
    assert(member_idx.size() != 0);
    assert(member_idx.size() <= members.size());

    unsigned int max_n_beats = 0;

    for (auto member : getMembersFromIdx(member_idx)) {
      if (max_n_beats < member->_n_beats) {
        max_n_beats = member->_n_beats;
      }
    }

    return max_n_beats;
  }

  inline unsigned int getNumberOfCircleBeats(TimePosition position) {
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
} // namespace kokopellivcv
