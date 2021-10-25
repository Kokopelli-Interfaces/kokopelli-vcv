#pragma once

#include "Timeline.hpp"
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

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct Engine {
  bool _use_ext_phase = false;
  float _ext_phase = 0.f;
  float _sample_time = 1.0f;

  // TODO make me an array to support MIX4 & PLAY
  kokopellivcv::dsp::SignalType _signal_type;

  std::vector<unsigned int> _selected_members_idx;
  std::vector<unsigned int> _saved_selected_members_idx;
  bool _new_member_active = true;
  bool _select_new_members = true;

  unsigned int _active_member_i;

  bool _member_mode = false;
  std::vector<unsigned int> _selected_members_idx_before_member_mode;
  std::pair<unsigned int, unsigned int> _circle_before_member_mode;

  /* read only */

  std::pair<unsigned int, unsigned int> _circle = std::make_pair(0, 1);

  Member *_recording_member = nullptr;
  RecordParams _record_params;

  PhaseOscillator _phase_oscillator;
  PhaseAnalyzer _phase_analyzer;

  Timeline _timeline;
  TimePosition _timeline_position;

  AntipopFilter _read_antipop_filter;
  AntipopFilter _write_antipop_filter;

  Options _options;

  void step();
  float read();
  float readSelection();
  float readActiveMember();

  void setCircleToMember(unsigned int member_i);
  void skipToActiveMember();

  void next();
  void forget();
  bool isRecording();

  void toggleFixBounds();
  void prev();
  void toggleMemberMode();

  void nextMember();
  void prevMember();
  void selectRange(unsigned int member_i_1, unsigned int member_i_2);
  void soloSelectMember(unsigned int member_i);
  bool isSelected(unsigned int member_i);
  bool isSolo(unsigned int member_i);
  void toggleSelectMember(unsigned int member_i);
  void toggleSelectActiveMember();
  void soloOrSelectUpToActiveMember();
  void deleteMember(unsigned int member_i);
  void deleteSelection();

private:
  void fitMemberIntoCircle(Member* member);
  void endRecording(bool loop, bool create_new_circle);
  Member* newRecording();
  inline bool phaseDefined();
  inline void write();
  inline void handleBeatChange(PhaseAnalyzer::PhaseEvent flip);
  inline PhaseAnalyzer::PhaseEvent advanceTimelinePosition();
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
