#pragma once

#include "Voice.hpp"
#include "definitions.hpp"

#include <vector>

namespace kokopellivcv {
namespace dsp {
namespace hearth {

class GroupPeriodChanger {
public:
  static inline std::vector<int> getSnapBeats(Time group_period, Time group_beat_period) {
    assert(group_beat_period != 0.f);

    std::vector<int> snap_beats;
    int beat = 0;
    int total_beats = group_period / group_beat_period;
    while(beat < total_beats) {
      beat++;
      if (total_beats % beat == 0) {
        snap_beats.push_back(beat);
      }
    }

    return snap_beats;
  }

  static inline Time getAdjustedPeriod(Time voice_period, Time group_period, Time group_beat_period) {
    Time adjusted_period;

    Time voice_period_diff = voice_period - group_period;
    if (0.f < voice_period_diff) {
      Time start_period = group_period;
      Time new_period = start_period;
      Time percent_over = voice_period / group_period;

      float period_round_back_percent = 1.5f;
      while (period_round_back_percent <= percent_over) {
        new_period += start_period;
        percent_over = percent_over - 1.f;
      }

      adjusted_period = new_period;
    } else {
      Time voice_time_in_beats = voice_period / group_beat_period;
      std::vector<int> snap_beats = getSnapBeats(group_period, group_beat_period);
      int snap_beat = 1;
      for (unsigned int movement_i = 1; movement_i < snap_beats.size(); movement_i++) {
        bool voice_is_between_beats = (float)snap_beats[movement_i-1] <= voice_time_in_beats && voice_time_in_beats <= (float)snap_beats[movement_i];
        if (voice_is_between_beats) {
          Time time_between_snap_beats = group_beat_period * (snap_beats[movement_i] - snap_beats[movement_i-1]);
          Time voice_position_in_between_snap_beats_area = voice_period - (group_beat_period * snap_beats[movement_i-1]);
          Time percent_position_in_snap_beats_area = voice_position_in_between_snap_beats_area / time_between_snap_beats;

          if (percent_position_in_snap_beats_area <= 0.5f) {
            snap_beat = snap_beats[movement_i-1];
          } else {
            snap_beat = snap_beats[movement_i];
          }
          break;
        }
      }

      Time snap_beat_time = (long double)snap_beat * group_beat_period;
      adjusted_period = snap_beat_time;
    }

    return adjusted_period;
  }

  static inline void adjustVoiceAndGroupPeriodsForNewLoopingVoice(Time &voice_period, Time &group_period, Time group_beat_period) {
    Time adjusted_period = getAdjustedPeriod(voice_period, group_period, group_beat_period);
    if (group_period < adjusted_period) {
      group_period = adjusted_period;
    }

    voice_period = adjusted_period;
  }

  static inline void fixVoicePlayheadAfterPeriodAdjustment(Time original_voice_period, Time original_voice_playhead, Voice *voice) {
    Time voice_period_diff = original_voice_period - voice->period;
    bool period_roundback = 0.f < voice_period_diff;
    if (period_roundback) {
      voice->playhead = voice_period_diff;
    } else {
      voice->playhead = original_voice_playhead;
    }
  }

  static inline void magicAdjustNewVoicePeriodAndGroupPeriodToPreserveSynchronization(Voice* voice, Time &group_period, Time &group_beat_period) {
    assert(voice->loop);

    Time original_voice_playhead = voice->playhead;
    Time original_voice_period = voice->period;
    Time original_group_period = group_period;

    adjustVoiceAndGroupPeriodsForNewLoopingVoice(voice->period, group_period, group_beat_period);
    printf("-- group period from (%Lf -> %Lf)\n", original_group_period, group_period);
    printf("-- voice period from (%Lf -> %Lf)\n", original_voice_period, voice->period);

    fixVoicePlayheadAfterPeriodAdjustment(original_voice_period, original_voice_playhead, voice);
    printf("-- fix voice playhead by %Lf ((original)%Lf - (new)%Lf)\n", voice->playhead - original_voice_playhead, original_voice_playhead, voice->playhead);
  }
};

} // namespace hearth
} // namespace dsp
} // namespace kokopellivcv
