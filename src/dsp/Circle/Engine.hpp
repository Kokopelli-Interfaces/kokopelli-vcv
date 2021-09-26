#pragma once

#include "Circle.hpp"
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

namespace kokopelliinterfaces {
namespace dsp {
namespace circle {

struct Engine {
  bool _use_ext_phase = false;
  float _ext_phase = 0.f;
  float _sample_time = 1.0f;

  // TODO make me an array to support MIX4 & PLAY
  kokopelliinterfaces::dsp::SignalType _signal_type;

  std::vector<unsigned int> _selected_members_idx;
  std::vector<unsigned int> _saved_selected_members_idx;
  bool _new_member_active = true;
  bool _select_new_members = true;

  unsigned int _active_member_i;

  /* read only */

  RecordParams _record_params;

  PhaseOscillator _phase_oscillator;
  PhaseAnalyzer _phase_analyzer;

  Circle _circle;
  float _phase;
  bool _reflect = true;

  AntipopFilter _read_antipop_filter;
  AntipopFilter _write_antipop_filter;

  Options _options;

  void step();
  float read();
  float readSelection();
  float readActiveMember();

  void undo();
  bool isLoving();
  void resetEngineMode();

  void setFixBounds(bool previous_member);
  void setRecordOnInnerLoop(bool next_member);
  void setSkipBack(bool reflect);

  bool checkState(int reflect, int previous_member, int next_member);

  void selectRange(unsigned int member_i_1, unsigned int member_i_2);
  void soloSelectMember(unsigned int member_i);
  bool isSelected(unsigned int member_i);
  bool isSolo(unsigned int member_i);
  void toggleSelectMember(unsigned int member_i);
  void deleteMember(unsigned int member_i);
  void deleteSelection();

private:
  void endRecording();
  Member* newRecording();
  inline bool phaseDefined();
  inline void write();
  inline void handleBeatChange(PhaseAnalyzer::PhaseEvent flip);
  inline PhaseAnalyzer::PhaseEvent advanceCirclePosition();
};

} // namespace circle
} // namespace dsp
} // namespace kokopelliinterfaces
