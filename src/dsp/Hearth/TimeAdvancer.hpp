#pragma once

#include "definitions.hpp"
#include "Village.hpp"
#include "Voice.hpp"
#include "Group.hpp"
#include "rack.hpp"
#include <math.h>

namespace kokopellivcv {
namespace dsp {
namespace hearth {

struct TimeAdvancer {
  float sample_time = 1.0f;
  bool use_ext_phase = false;
  float ext_phase = 0.f;

  float _last_ext_phase = 0.f;

  float _freq = 0.f;
  bool _freq_set = false;

  // TODO performance improvement option
  // rack::dsp::ClockDivider _advance_groups_playhead_divider;

  TimeAdvancer() {}

  inline void setTickFrequency(float freq) {
    _freq = freq;
    _freq_set = true;
  }

  inline bool isSet() {
    return _freq_set;
  }

  inline float getTickFrequency() {
    return _freq;
  }

  // NOTE that group playheads are advanced independantly
  inline void scrubGroup(Group *group, Time time) {
    group->playhead += time;

    while (group->_period <= group->playhead) {
      group->playhead -= group->_period;
    }
  }

  inline void scrubVoice(Voice *voice, Time time) {
    // TODO RACE condition if PLAYHEAD is altered somewhere else
    voice->playhead += time;
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
    }
  }

  inline void advanceVoice(Voice *voice, float dt) {
    scrubVoice(voice, _freq * dt);
  }

  inline void advanceGroup(Group *group, float dt) {
    if (group->_period == 0.f) {
      return;
    }
    scrubGroup(group, _freq * dt);
  }

  inline void advanceTime(Village &village) {
    float dt = this->sample_time;
    // FIXME
    // if (this->use_ext_phase) {
    //   dt = this->ext_phase - _last_ext_phase;
    //   if (dt < -0.95f) {
    //     dt = this->ext_phase + 1 - _last_ext_phase;
    //   } else if (0.95f < dt) {
    //     dt = this->ext_phase - 1 - _last_ext_phase;
    //   }
    //   _last_ext_phase = this->ext_phase;
    // }

    // new voice does not have period set until finish
    village.new_voice->playhead += _freq * dt;

    for (Voice* voice : village.voices) {
      advanceVoice(voice, dt);
    }

    // TODO if _advance_groups_playhead_divider)
    for (Group* group : village.groups) {
      advanceGroup(group, dt);
    }
  }
};

} // namespace hearth
} // namespace dsp
} // namespace kokopellivcv
