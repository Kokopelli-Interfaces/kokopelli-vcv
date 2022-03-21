#pragma once

#include <vector>
#include <numeric> // std::iota

#include "rack.hpp"

#include "Observer.hpp"
#include "Conductor.hpp"
#include "LoveUpdater.hpp"
#include "OutputUpdater.hpp"
#include "Village.hpp"
#include "Voice.hpp"
#include "Group.hpp"
#include "definitions.hpp"
#include "Movement.hpp"
#include "TimeAdvancer.hpp"
#include "util/math.hpp"
#include "dsp/Signal.hpp"

namespace kokopellivcv {
namespace dsp {
namespace circle {

class Gko {
public:
  bool use_ext_phase = false;
  float ext_phase = 0.f;

  float sample_time = 1.0f;
  Time delay_shiftback = 0.f;
  bool tune_to_frequency_of_observed_sun = true;

  /** read only */

  Conductor conductor;
  Observer observer;
  LoveUpdater love_updater;
  OutputUpdater output_updater;

  bool _discard_voice_at_next_love_return = false;

  float _last_ext_phase = 0.f;
  TimeAdvancer _time_advancer;

  LoveDirection _love_direction;

public:
  Gko() {
    _time_advancer.setTickFrequency(1.0f);
    // TODO set me when loop is observed_sun for consistent loops
  }

private:
  inline void addVoice(Village &village, Voice* ended_voice) {
    village.voices.push_back(ended_voice);
    ended_voice->immediate_group->addNewVoice(ended_voice);
    if (delay_shiftback < ended_voice->period) {
      ended_voice->playhead += delay_shiftback;
    }
  }

public:
  inline void nextVoice(Village &village, VoiceEnd voice_end) {
    Voice* ended_voice = village.new_voice;

    // may happen when reverse recording
    ended_voice->finishWrite();
    if (ended_voice->period == 0.0) {
      delete ended_voice;
      village.new_voice = new Voice(village.playhead, village.current_movement, village.observed_sun);
      return;
    }

    switch (voice_end) {
    case VoiceEnd::DISCARD:
      village.clearEmptyGroups();
      delete ended_voice;
      break;
    case VoiceEnd::NEXT_MOVEMENT_VIA_SHIFT:
      // ended_voice->loop = false;
      // village.voices.push_back(ended_voice);
      // ended_voice->immediate_group->addNewVoice(ended_voice);
      conductor.nextMovement(village);
      delete ended_voice;
      break;
    case VoiceEnd::JOIN_OBSERVED_SUN_LOOP:
      ended_voice->loop = true;
      addVoice(village, ended_voice);
      break;
    case VoiceEnd::SET_EQUAL_PERIOD_AND_JOIN_OBSERVED_SUN_LOOP:
      ended_voice->loop = true;
      _discard_voice_at_next_love_return = true;
      if (ended_voice->immediate_group->period != 0.f) {
        ended_voice->setPeriodToCaptureWindow(ended_voice->immediate_group->period);
      }
      addVoice(village, ended_voice);
      break;
    case VoiceEnd::FLOOD:
      for (int i = village.voices.size()-1; 0 <= i; i--) {
        if (Observer::checkIfVoiceInGroupOneIsObservedByVoiceInGroupTwo(village.voices[i]->immediate_group, ended_voice->immediate_group)) {
          village.voices[i]->immediate_group->undoLastVoice();
          village.voices.erase(village.voices.begin() + i);
        }
      }

      // works better without

      _discard_voice_at_next_love_return = true;

      delete ended_voice;
      break;
    }

    // TODO
    Movement* voice_movement;
    if (tune_to_frequency_of_observed_sun) {
      voice_movement = village.current_movement;
    } else {
      // FIXME next movement
      voice_movement = village.current_movement;
    }

    village.new_voice = new Voice(village.playhead, voice_movement, village.observed_sun);
  }

  inline void undoVoice(Village &village) {
    if (observer.checkIfInSubgroupMode()) {
      observer.exitSubgroupMode(village);
    }

    if (0 < village.voices.size()) {
      Voice* most_recent_voice = village.voices[village.voices.size()-1];
      most_recent_voice->immediate_group->undoLastVoice();
      village.voices.pop_back();
    }

    nextVoice(village, VoiceEnd::DISCARD);
  }

  inline void voiceBackward(Village &village) {
    switch(_love_direction) {
    case LoveDirection::OBSERVED_SUN:
      if (observer.checkIfInSubgroupMode()) {
        if (observer.checkIfCanEnterFocusedSubgroup()) {
          observer.exitSubgroupMode(village);
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

  inline void voiceForward(Village &village) {
    switch(_love_direction) {
    case LoveDirection::OBSERVED_SUN:
      if (observer.checkIfInSubgroupMode()) {
        if (observer.checkIfCanEnterFocusedSubgroup()) {
          observer.exitSubgroupMode(village);
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

  inline void voiceObservation(Village &village) {
    switch(_love_direction) {
    case LoveDirection::OBSERVED_SUN:
      if (!observer.checkIfInSubgroupMode()) {
        observer.tryEnterSubgroupMode(village);
      } else {
        observer.voicesubgroup(village);
      }
      nextVoice(village, VoiceEnd::DISCARD);
      break;
    case LoveDirection::EMERGENCE:
    case LoveDirection::NEW:
      nextVoice(village, VoiceEnd::SET_EQUAL_PERIOD_AND_JOIN_OBSERVED_SUN_LOOP);
      break;
    }
  }

  inline void handleLoveDirectionChange(Village &village, LoveDirection new_love_direction) {
    assert(new_love_direction != _love_direction);

    if (new_love_direction == LoveDirection::OBSERVED_SUN) {
      if (_discard_voice_at_next_love_return) {
        nextVoice(village, VoiceEnd::DISCARD);
        _discard_voice_at_next_love_return = false;
      } else {
        nextVoice(village, VoiceEnd::JOIN_OBSERVED_SUN_LOOP);
      }

      if (observer.checkIfInSubgroupMode()) {
        observer.exitSubgroupMode(village);
      }
    } else if (_love_direction == LoveDirection::OBSERVED_SUN && new_love_direction == LoveDirection::EMERGENCE) {
      nextVoice(village, VoiceEnd::DISCARD);
    }

    _love_direction = new_love_direction;
  }

  inline void advanceTime(Village &village) {
    float step = this->sample_time;
    if (use_ext_phase) {
      step = ext_phase - _last_ext_phase;
      if (step < -0.95f) {
        step = ext_phase + 1 - _last_ext_phase;
      } else if (0.95f < step) {
        step = ext_phase - 1 - _last_ext_phase;
      }
      _last_ext_phase = ext_phase;
    }

    if (!village.voices.empty()) {
      _time_advancer.step(village.playhead, step);
    } else {
      village.playhead = 0.f;
    }

    _time_advancer.step(village.new_voice->playhead, step);
    if (use_ext_phase) {
      if (village.playhead < 0.f) {
        village.playhead = 0.f;
      }
      if (village.new_voice->playhead < 0.f) {
        village.new_voice->playhead = 0.f;
      }
    }

    for (Voice* voice : village.voices) {
      _time_advancer.step(voice->playhead, step);
      if (voice->period < voice->playhead) {
        while (voice->period < voice->playhead) {
        // printf("advanceTime: skip back voice (%Lf < %Lf)\n", voice->period, voice->playhead);
          voice->playhead -= voice->period;
        }
      } else if (voice->playhead < 0.f) {
        while (voice->playhead < 0.f) {
        // printf("advanceTime: skip back voice (%Lf < %Lf)\n", voice->period, voice->playhead);
          voice->playhead += voice->period;
        }
        voice->playhead += voice->period;
      }

      assert(0 <= voice->playhead);
    }
  }

public:
  inline void advance(Village &village, Inputs inputs, Options options) {
    advanceTime(village);
    village.new_voice->write(inputs.in, inputs.love);

    LoveDirection new_love_direction = Inputs::getLoveDirection(inputs.love);
    if (_love_direction != new_love_direction) {
      handleLoveDirectionChange(village, new_love_direction);
    }

    love_updater.updateVillageVoicesLove(village.voices);
    output_updater.updateOutput(village.out, village.voices, village.new_voice->immediate_group, inputs, options);

    conductor.conduct(village);
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
