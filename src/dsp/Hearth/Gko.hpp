#pragma once

#include <vector>

#include "rack.hpp"

#include "LoveUpdater.hpp"
#include "GroupChanger.hpp"
#include "ConductorChanger.hpp"
#include "OutputUpdater.hpp"
#include "Village.hpp"
#include "Voice.hpp"
#include "Group.hpp"
#include "definitions.hpp"
#include "TimeAdvancer.hpp"
#include "util/math.hpp"
#include "dsp/Signal.hpp"

namespace kokopellivcv {
namespace dsp {
namespace hearth {

// The Advancer of the Village
class Gko {
public:
  Time delay_shiftback = 0.f;
  bool tune_to_frequency_of_focus_group = true;

  TimeAdvancer time_advancer;

  /** read only */
  LoveUpdater _love_updater;

  bool _discard_voice_at_next_love_return = false;
  LoveDirection _love_direction;

public:
  Gko() {
    time_advancer.setTickFrequency(1.0f);
    // TODO set me when loop is focus_group for consistent loops
  }

private:
  inline void addLoopingVoice(Village *village, Voice* ended_voice) {
    village->voices.push_back(ended_voice);
    GroupChanger::addNewLoopingVoice(ended_voice->immediate_group, ended_voice);
    if (delay_shiftback < ended_voice->period) {
      ended_voice->playhead += delay_shiftback;
    }
  }

public:
  inline void nextVoice(Village *village, VoiceEnd voice_end) {
    Voice* ended_voice = village->new_voice;

    // may happen when reverse recording
    ended_voice->finishWrite();
    if (ended_voice->period == 0.0) {
      delete ended_voice;
      village->new_voice = new Voice(village->conductor.focus_group);
      return;
    }

    switch (voice_end) {
    case VoiceEnd::DISCARD:
      village->clearEmptyGroups();
      delete ended_voice;
      break;
    case VoiceEnd::NEXT_MOVEMENT_VIA_SHIFT:
      // ended_voice->loop = false;
      // village->voices.push_back(ended_voice);
      // ended_voice->immediate_group->addNewLoopingVoice(ended_voice);

      // TODO _conductor.nextMovement(village);

      delete ended_voice;
      break;
    case VoiceEnd::JOIN_FOCUS_GROUP_LOOP:
      ended_voice->loop = true;
      addLoopingVoice(village, ended_voice);
      break;
    case VoiceEnd::SET_EQUAL_PERIOD_AND_JOIN_FOCUS_GROUP_LOOP:
      ended_voice->loop = true;
      _discard_voice_at_next_love_return = true;
      if (ended_voice->immediate_group->period != 0.f) {
        ended_voice->setPeriodToCaptureWindow(ended_voice->immediate_group->period);
      }
      addLoopingVoice(village, ended_voice);
      break;
    case VoiceEnd::FLOOD:
      for (int i = village->voices.size()-1; 0 <= i; i--) {
        if (ConductorChanger::checkIfVoiceInGroupOneIsObservedByVoiceInGroupTwo(village->voices[i]->immediate_group, ended_voice->immediate_group)) {
          GroupChanger::undoLastVoice(village->voices[i]->immediate_group);
          village->voices.erase(village->voices.begin() + i);
        }
      }

      if (village->conductor.is_choosing_next_group) {
        ConductorChanger::exitSubgroupMode(&village->conductor, village->groups);
      }

      // works better without
      _discard_voice_at_next_love_return = true;

      delete ended_voice;
      break;
    }

    // // TODO
    // Movement* voice_movement;
    // if (tune_to_frequency_of_focus_group) {
    //   voice_movement = village->current_movement;
    // } else {
    //   // FIXME next movement
    //   voice_movement = village->current_movement;
    // }

    village->new_voice = new Voice(village->conductor.focus_group);
  }

  inline void undoVoice(Village *village) {
    if (village->conductor.is_choosing_next_group) {
      ConductorChanger::exitSubgroupMode(&village->conductor, village->groups);
    }

    if (0 < village->voices.size()) {
      Voice* most_recent_voice = village->voices[village->voices.size()-1];
                GroupChanger::undoLastVoice(most_recent_voice->immediate_group);
      village->voices.pop_back();
    }

    nextVoice(village, VoiceEnd::DISCARD);
  }

  inline void cycleBackward(Village *village) {
    switch(_love_direction) {
    case LoveDirection::FOCUS_GROUP:
      // nextVoice(village, VoiceEnd::PREV_MOVEMENT_VIA_SHIFT);

      // if (village->conductor.is_choosing_next_group) {
      //   if (ConductorChanger::checkIfCanEnterFocusedSubgroup()) {
      //     ConductorChanger::exitSubgroupMode(village->conductor, village->groups);
      //   }
      //   nextVoice(village, VoiceEnd::DISCARD);
      // } else {
        // nextVoice(village, VoiceEnd::PREV_MOVEMENT_VIA_SHIFT);
      // }
      break;
    case LoveDirection::EMERGENCE:
      // nextVoice(village, VoiceEnd::NO_LOOP_AND_GO_TO_MOVEMENT_START)
      break;
    case LoveDirection::NEW:
      // nextVoice(village, VoiceEnd::FLOOD);
      break;
    }
  }

  inline void cycleForward(Village *village) {
    switch(_love_direction) {
    case LoveDirection::FOCUS_GROUP:
      if (village->conductor.is_choosing_next_group) {
        if (village->conductor.checkIfCanEnterFocusedSubgroup()) {
          ConductorChanger::exitSubgroupMode(&village->conductor, village->groups);
        }
        nextVoice(village, VoiceEnd::DISCARD);
      } else {
        nextVoice(village, VoiceEnd::NEXT_MOVEMENT_VIA_SHIFT);
      }
      break;
    case LoveDirection::EMERGENCE:
      nextVoice(village, VoiceEnd::DISCARD);
      break;
    case LoveDirection::NEW:
      nextVoice(village, VoiceEnd::FLOOD);
      break;
    }
  }

  inline void cycleObservation(Village *village) {
    switch(_love_direction) {
    case LoveDirection::FOCUS_GROUP:
      if (!village->conductor.is_choosing_next_group) {
        ConductorChanger::tryEnterChooseNextGroupMode(&village->conductor, village->groups);
      } else {
        ConductorChanger::selectNextPotentialFocusGroup(&village->conductor, village->groups);
      }
      nextVoice(village, VoiceEnd::DISCARD);
      break;
    case LoveDirection::EMERGENCE:
    case LoveDirection::NEW:
      nextVoice(village, VoiceEnd::SET_EQUAL_PERIOD_AND_JOIN_FOCUS_GROUP_LOOP);
      break;
    }
  }

  inline void handleLoveDirectionChange(Village *village, LoveDirection new_love_direction) {
    assert(new_love_direction != _love_direction);

    if (new_love_direction == LoveDirection::FOCUS_GROUP) {
      if (_discard_voice_at_next_love_return) {
        nextVoice(village, VoiceEnd::DISCARD);
        _discard_voice_at_next_love_return = false;
      } else {
        nextVoice(village, VoiceEnd::JOIN_FOCUS_GROUP_LOOP);
      }

      if (village->conductor.is_choosing_next_group) {
        ConductorChanger::exitSubgroupMode(&village->conductor, village->groups);
      }
    } else if (_love_direction == LoveDirection::FOCUS_GROUP && new_love_direction == LoveDirection::EMERGENCE) {
      nextVoice(village, VoiceEnd::DISCARD);
    }

    _love_direction = new_love_direction;
  }

public:
  inline void resetState(Village *village) {
    nextVoice(village, VoiceEnd::DISCARD);
    _love_updater.resetState();
  }

  inline void toggleMovementProgression() {
    // FIXME
    return;
  }

  inline void ascend(Village *village) {
    ConductorChanger::ascend(village);
    nextVoice(village, VoiceEnd::DISCARD);
    _discard_voice_at_next_love_return = true;
  }

  inline void advance(Village *village, Inputs inputs, Options options) {
    time_advancer.advanceTime(village);
    village->new_voice->write(inputs.in, inputs.love);

    LoveDirection new_love_direction = Inputs::getLoveDirection(inputs.love);
    if (_love_direction != new_love_direction) {
      handleLoveDirectionChange(village, new_love_direction);
    }

    _love_updater.updateVillageVoicesLove(village->voices);
    OutputUpdater::updateOutput(village->out, village->voices, village->new_voice->immediate_group, inputs, options);
  }
};

} // namespace hearth
} // namespace dsp
} // namespace kokopellivcv
