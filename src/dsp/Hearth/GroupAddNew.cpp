#include "Group.hpp"

using namespace kokopellivcv::dsp::hearth;
using namespace kokopellivcv::dsp;

std::vector<int> getSnapBeats(Time group_period, Time group_beat_period) {
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

Time getAdjustedPeriod(Time voice_period, Time group_period, Time group_beat_period) {
  Time adjusted_period;

  Time period_diff = voice_period - group_period;
  if (0.f < period_diff) {
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

void adjustVoiceAndGroupPeriodsForNewLoopingVoice(Time &voice_period, Time &group_period, Time group_beat_period) {
  Time adjusted_period = getAdjustedPeriod(voice_period, group_period, group_beat_period);
  if (group_period < adjusted_period) {
    group_period = adjusted_period;
  }

  voice_period = adjusted_period;
}

void adjustVoiceAndGroupPeriodsForNewLoopingVoiceAndCorrectPlayhead(Voice* voice, Time &group_period, Time &group_beat_period) {
  // TODO can't change after.
  assert(voice->loop);

  Time original_playhead = voice->playhead;
  Time original_period = voice->period;

  Time period_before = voice->period;
  adjustVoiceAndGroupPeriodsForNewLoopingVoice(voice->period, group_period, group_beat_period);
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

void Group::addNewLoopingVoice(Voice* voice) {
  assert(voice->loop);

  if (this->parent_group != nullptr && voice->immediate_group != this->parent_group) {
    this->parent_group->addNewLoopingVoice(voice);
  }

  _voices_in_group.push_back(voice);
  _next_voices_relative_love.push_back(1.f);
  _period_history.push_back(_period);

  if (_voices_in_group.size() == 1) {
    _period = voice->period;
    _beat_period = voice->period;
  } else {
    adjustVoiceAndGroupPeriodsForNewLoopingVoiceAndCorrectPlayhead(voice, _period, _beat_period);
  }

  printf("-- added to group %s, n_beats->%d\n", this->name.c_str(), getBeatN());
  printf("-- voice group_period %Lf, voice playhead %Lf\n", voice->period, voice->playhead);
}

void Group::addExistingVoice(Voice* voice) {
  _voices_in_group.push_back(voice);
  _next_voices_relative_love.push_back(1.f);
  _period_history.push_back(_period);

  Time period_before = voice->period;
  if (voice->loop) {
    if (_voices_in_group.size() == 1) {
      _period = voice->period;
      _beat_period = voice->period;
    } else {
      adjustVoiceAndGroupPeriodsForNewLoopingVoice(voice->period, _period, _beat_period);
    }
  }
  assert(voice->period == period_before);

  printf("-- add existing voice to group %s, n_beats->%d\n", this->name.c_str(), getBeatN());
}
