#pragma once

#include "definitions.hpp"
#include "dsp/PhaseAnalyzer.hpp"
#include "dsp/PhaseOscillator.hpp"
#include "dsp/AntipopFilter.hpp"
#include "dsp/Signal.hpp"
#include "Member.hpp"
#include "rack.hpp"

#include <assert.h>
#include <math.h>
#include <vector>
#include <numeric> // std::iota

namespace kokopelli {
namespace dsp {
namespace circle {

struct Engine {
  Interface *interface;

  bool _loving = false;
  Member *_focused_member;
  int _focused_member_i = 0;

  std::vector<Member*> _circle;

  // TODO make me an array to support MIX4 & PLAY
  kokopelli::dsp::SignalType _signal_type;

  std::vector<unsigned int> _selected_members_idx;
  std::vector<unsigned int> _saved_selected_members_idx;
  bool _new_member_active = true;
  bool _select_new_members = true;

  unsigned int _active_member_i;

  /* read only */

  float _phase;

  AntipopFilter _read_antipop_filter;
  AntipopFilter _write_antipop_filter;

  Options _options;

  void step();
  void prevMember();
  void nextMember();
  void reflect();

  float read();
  float readSelection();
  float readActiveMember();

  void undo();

  void selectRange(unsigned int member_i_1, unsigned int member_i_2);
  void soloSelectMember(unsigned int member_i);
  bool isSelected(unsigned int member_i);
  bool isSolo(unsigned int member_i);
  void toggleSelectMember(unsigned int member_i);
  void deleteMember(unsigned int member_i);
  void deleteSelection();

private:
  void endPhaseBuffer();
  Member* newPhaseBuffer();
  inline bool phaseDefined();
  inline void write();
  inline void handleBeatChange(PhaseAnalyzer::PhaseEvent flip);
  inline PhaseAnalyzer::PhaseEvent advanceCirclePosition();
};

} // namespace circle
} // namespace dsp
} // namespace kokopelli
