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

  std::vector<Time> movements;
  std::vector<int> voice_i_to_movement_i;
  int current_movement_i = 0;

  std::vector<Voice*> voices;

  std::vector<Time> period_history;

  Time period = 0.0;
  Time beat_period = 0.0;

  inline float getMostRecentMovementPhase() {
    return 1.f;
  }

  inline unsigned int getMostRecentMovement(int offset) {
    if (this->movements.empty()) {
      return 0;
    }

    int index = current_movement_i + offset;
    while (index < 0) {
      index += this->movements.size();
    }
    return (index % this->movements.size()) + 1;
  }


  inline bool checkIfVoiceIsInGroup(Voice* voice) {
    for (Voice* voice_in_group : this->voices) {
      if (voice_in_group == voice) {
        return true;
      }
    }
    return false;
  }

  inline float getBeatPhase() {
    if (this->period != 0.0 && this->beat_period != 0.0) {
      Time time_in_beat = std::fmod((float)this->playhead, (float)this->beat_period);
      return rack::clamp(time_in_beat / this->beat_period , 0.f, 1.f);
    }
    return 0.f;
  }

  inline float getPhase() {
    if (this->period != 0.0 && this->beat_period != 0.0) {
      return rack::clamp(this->playhead / this->period , 0.f, 1.f);
    }
    return 0.f;
  }

  inline int getBeatN() {
    if (this->period != 0.f && this->beat_period != 0.f) {
      return (int) (this->playhead / this->beat_period);
    }
    return 0;
  }

  inline int convertToBeat(Time time, bool mod) {
    if (this->period != 0.f && this->beat_period != 0.f) {
      float time_in_focus_group = time;
      if (mod && this->period < time) {
        time_in_focus_group = std::fmod((float)time, (float)this->period);
      }

      return (int) (time_in_focus_group / this->beat_period);
    }
    return 0;
  }

  inline int getTotalBeats() {
    if (this->beat_period != 0.f) {
      return (this->period / this->beat_period);
    } else {
      return 0;
    }
  }
};

} // namespace hearth
} // namespace dsp
} // namespace kokopellivcv
