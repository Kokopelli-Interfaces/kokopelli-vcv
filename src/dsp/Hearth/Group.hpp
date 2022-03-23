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

  std::vector<Time> _movements;
  std::vector<int> _voice_i_to_movement_i;
  int _current_movement_i = 0;

  std::vector<Voice*> _voices_in_group;

  std::vector<float> _next_voices_relative_love;
  std::vector<Time> _period_history;

  Time _period = 0.0;
  Time _beat_period = 0.0;

private: // GroupInfo.cp
  void undoLastVoiceWithoutUndoingParent();
public:
  void undoLastVoice();
  bool checkIfVoiceIsInGroup(Voice* voice);
  float getBeatPhase();
  float getPhase();
  int getBeatN();
  int convertToBeat(Time time, bool mod);
  int getTotalBeats();

public: // GroupAddNew.cpp
  void addNewLoopingVoice(Voice* voice);
  void addExistingVoice(Voice* voice);

private: // GroupMovements.cpp
  void addVoiceToMovements(Voice *voice);
public:
  float getMostRecentMovementPhase();
  unsigned int getMostRecentMovement(int offset);

};
} // namespace hearth
} // namespace dsp
} // namespace kokopellivcv
