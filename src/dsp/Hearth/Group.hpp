#pragma once

#include "Voice.hpp"
#include "definitions.hpp"

#include <vector>

namespace kokopellivcv {
namespace dsp {
namespace hearth {

struct Group {
  Time playhead = 0.f;
  Group *parent_group = nullptr;
  std::string name = "A";

  std::vector<Movement*> movements;
  int movement_i = 0;

  std::vector<Voice*> _voices_in_group;
  std::vector<int> _voice_i_to_entrance_movement_i;

  std::vector<float> _next_voices_relative_love;
  std::vector<Time> _period_history;

  Time _period = 0.0;
  Time _beat_period = 0.0;

public:
  unsigned int getMostRecentMovement(int offset) {
    if (this->movements.empty()) {
      return 0;
    }

    int index = this->movement_i + offset;
    while (index < 0) {
      index += this->movements.size();
    }
    return (index % this->movements.size()) + 1;
  }

  // inline void growPeriodAndRippleUpwards(float new_period) {
  //   if (new_period < _period) {
  //     return;
  //   }

  //   assert(_period % _beat_period == 0);
  //   _period = new_period;
  //   if (this->parent_group) {
  //     this->parent_group->growPeriodAndRippleUpwards(new_period);
  //   }
  // }

private:
  inline void undoLastVoiceWithoutUndoingParent() {
    assert(_voices_in_group.size() == _period_history.size());

    int last_i = _voices_in_group.size() - 1;
    _voices_in_group.pop_back();
    _period = _period_history[last_i];
    _period_history.pop_back();
    _next_voices_relative_love.pop_back();
  }

public:
  inline void undoLastVoice() {
    undoLastVoiceWithoutUndoingParent();

    if (this->parent_group) {
      this->parent_group->undoLastVoice();
    }
  }

  inline bool checkIfVoiceIsInGroup(Voice* voice) {
    for (Voice* voice_in_group : _voices_in_group) {
      if (voice_in_group == voice) {
        return true;
      }
    }
    return false;
  }

  inline float getBeatPhase() {
    if (_period != 0.0 && _beat_period != 0.0) {
      Time time_in_beat = std::fmod((float)this->playhead, (float)_beat_period);
      return rack::clamp(time_in_beat / _beat_period , 0.f, 1.f);
    }
    return 0.f;
  }

  inline float getPhase() {
    if (_period != 0.0 && _beat_period != 0.0) {
      return rack::clamp(this->playhead / _period , 0.f, 1.f);
    }
    return 0.f;
  }

  inline int getBeatN() {
    if (_period != 0.f && _beat_period != 0.f) {
      return (int) (this->playhead / _beat_period);
    }
    return 0;
  }

  inline int convertToBeat(Time time, bool mod) {
    if (_period != 0.f && _beat_period != 0.f) {
      float time_in_observed_sun = time;
      if (mod && _period < time) {
        time_in_observed_sun = std::fmod((float)time, (float)_period);
      }

      return (int) (time_in_observed_sun / _beat_period);
    }
    return 0;
  }

  inline int getTotalBeats() {
    if (_beat_period != 0.f) {
      return (_period / _beat_period);
    } else {
      return 0;
    }
  }

private:
  inline std::vector<int> getSnapBeats() {
    assert(_beat_period != 0.f);

    std::vector<int> snap_beats;
    int beat = 0;
    int total_beats = _period / _beat_period;
    while(beat < total_beats) {
      beat++;
      if (total_beats % beat == 0) {
        snap_beats.push_back(beat);
      }
    }

    return snap_beats;
  }

  inline Time getAdjustedPeriod(Time voice_period) {
    assert(!_voices_in_group.empty());

    Time adjusted_period;

    Time period_diff = voice_period - _period;
    if (0.f < period_diff) {
      Time start_period = _period;
      Time new_period = start_period;
      Time percent_over = voice_period / _period;

      float period_round_back_percent = 1.5f;
      while (period_round_back_percent <= percent_over) {
        new_period += start_period;
        percent_over = percent_over - 1.f;
      }

      adjusted_period = new_period;
    } else {
      Time voice_time_in_beats = voice_period / _beat_period;
      std::vector<int> snap_beats = getSnapBeats();
      int snap_beat = 1;
      for (unsigned int movement_i = 1; movement_i < snap_beats.size(); movement_i++) {
        bool voice_is_between_beats = (float)snap_beats[movement_i-1] <= voice_time_in_beats && voice_time_in_beats <= (float)snap_beats[movement_i];
        if (voice_is_between_beats) {
          Time time_between_snap_beats = _beat_period * (snap_beats[movement_i] - snap_beats[movement_i-1]);
          Time voice_position_in_between_snap_beats_area = voice_period - (_beat_period * snap_beats[movement_i-1]);
          Time percent_position_in_snap_beats_area = voice_position_in_between_snap_beats_area / time_between_snap_beats;

          if (percent_position_in_snap_beats_area <= 0.5f) {
            snap_beat = snap_beats[movement_i-1];
          } else {
            snap_beat = snap_beats[movement_i];
          }
          break;
        }
      }

      Time snap_beat_time = (long double)snap_beat * _beat_period;
      adjusted_period = snap_beat_time;
    }

    return adjusted_period;
  }

  inline void adjustVoiceAndGroupPeriodsForNewLoopingVoice(Time &voice_period) {
    assert(!_voices_in_group.empty());

    Time adjusted_period = getAdjustedPeriod(voice_period);
    if (_period < adjusted_period) {
      _period = adjusted_period;
    }

    voice_period = adjusted_period;
  }

  inline void adjustVoiceAndGroupPeriodsForNewLoopingVoiceAndCorrectPlayhead(Voice* voice) {
    // TODO can't change after.
    assert(voice->loop);

    Time original_playhead = voice->playhead;
    Time original_period = voice->period;

    if (_voices_in_group.size() == 1) {
      // TODO get crossfade from prev voice
      _period = voice->period;
      _beat_period = voice->period;
    } else {
      Time period_before = voice->period;
      adjustVoiceAndGroupPeriodsForNewLoopingVoice(voice->period);
      printf("-- voice period from (%Lf -> %Lf) (original %Lf)\n", period_before, voice->period, original_period);
      Time period_diff = period_before - voice->period;
      bool period_roundback = 0.f < period_diff;
      if (period_roundback) {
        voice->playhead = period_diff;
        printf("-- move playhead to %Lf (0 < (original)%Lf - (new)%Lf)\n", period_diff, period_before, voice->period);
      } else {
        voice->playhead = original_playhead;
        printf("-- move playhead to original spot %Lf ((original)%Lf - (new)%Lf < 0)\n", original_playhead, period_before, voice->period);
      }
    }
  }

inline void updateMovements(Voice* voice) {
}

public:
  inline void addNewLoopingVoice(Voice* voice) {
    assert(voice->loop);

    if (this->parent_group != nullptr && voice->immediate_group != this->parent_group) {
      this->parent_group->addNewLoopingVoice(voice);
    }

    _voices_in_group.push_back(voice);
    _next_voices_relative_love.push_back(1.f);
    _period_history.push_back(_period);

    adjustVoiceAndGroupPeriodsForNewLoopingVoiceAndCorrectPlayhead(voice);

    printf("-- added to group %s, n_beats->%d\n", this->name.c_str(), getBeatN());
    printf("-- voice _period %Lf, voice playhead %Lf\n", voice->period, voice->playhead);
  }

  inline void addExistingVoice(Voice* voice) {
    _voices_in_group.push_back(voice);
    _next_voices_relative_love.push_back(1.f);
    _period_history.push_back(_period);

    Time period_before = voice->period;
    if (voice->loop) {
      if (_voices_in_group.size() == 1) {
        _period = voice->period;
        _beat_period = voice->period;
      } else {
        adjustVoiceAndGroupPeriodsForNewLoopingVoice(voice->period);
      }
    }
    assert(voice->period == period_before);

    printf("-- add existing voice to group %s, n_beats->%d\n", this->name.c_str(), getBeatN());
  }
};

} // namespace hearth
} // namespace dsp
} // namespace kokopellivcv
