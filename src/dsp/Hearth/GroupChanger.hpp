#pragma once

#include "Voice.hpp"
#include "Group.hpp"
#include "definitions.hpp"
#include "GroupPeriodChanger.hpp"
#include "GroupMovementChanger.hpp"

#include <vector>

namespace kokopellivcv {
namespace dsp {
namespace hearth {

class GroupChanger {
public:
  static inline void assertInvariants(Group* group) {
    assert(group->voices.size() == group->period_history.size());
    assert(group->voices.size() == group->voice_i_to_movement_i.size());
  }

  static inline void undoLastVoiceWithoutUndoingParent(Group* group) {
    int last_i = group->voices.size() - 1;
    group->voices.pop_back();
    group->period = group->period_history[last_i];
    group->period_history.pop_back();
    group->voice_i_to_movement_i.pop_back();
  }

  static inline void undoLastVoice(Group* group) {
    assertInvariants(group);

    undoLastVoiceWithoutUndoingParent(group);

    if (group->parent_group) {
      undoLastVoice(group->parent_group);
    }
  }

  static inline void addNewLoopingVoice(Group* group, Voice* voice) {
    assert(voice->loop);

    if (group->parent_group != nullptr && voice->immediate_group != group->parent_group) {
      addNewLoopingVoice(group->parent_group, voice);
    }

    group->voices.push_back(voice);
    group->period_history.push_back(group->period);

    if (group->voices.size() == 1) {
      group->period = voice->period;
      group->beat_period = voice->period;
    } else {
      GroupPeriodChanger::magicAdjustNewVoicePeriodAndGroupPeriodToPreserveSynchronization(voice, group->period, group->beat_period);
    }

    GroupMovementChanger::addVoiceToMovements(group, voice);

    printf("-- added voice to group %s, n_beats->%d\n", group->name.c_str(), group->getBeatN());
    printf("-- group period %Lf, voice group->period %Lf, voice playhead %Lf\n", group->period, voice->period, voice->playhead);
  }

  static inline void addExistingVoice(Group* group, Voice* voice) {
    group->voices.push_back(voice);
    group->period_history.push_back(group->period);

    Time period_before = voice->period;
    if (voice->loop) {
      if (group->voices.size() == 1) {
        group->period = voice->period;
        group->beat_period = voice->period;
      } else {
        // TODO or is it MAGIC ?
        GroupPeriodChanger::adjustVoiceAndGroupPeriodsForNewLoopingVoice(voice->period, group->period, group->beat_period);
      }
    }
    assert(voice->period == period_before);

    printf("-- add existing voice to group %s, n_beats->%d\n", group->name.c_str(), group->getBeatN());
  }
};

} // namespace hearth
} // namespace dsp
} // namespace kokopellivcv
