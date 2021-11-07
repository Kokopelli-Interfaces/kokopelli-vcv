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
  kokopellivcv::dsp::SignalType _signal_type = kokopellivcv::dsp::SignalType::AUDIO;

  std::vector<unsigned int> _selected_members_idx;
  std::vector<unsigned int> _saved_selected_members_idx;

  unsigned int _focused_member_i;

  LoveDirection _love_direction = LoveDirection::ESTABLISHED;

  bool _member_mode = false;
  std::vector<unsigned int> _selected_members_idx_before_member_mode;
  std::pair<unsigned int, unsigned int> _circle_before_member_mode;

  /* read only */

  std::pair<unsigned int, unsigned int> _circle = std::make_pair(0, 1);

  bool _tune_to_frequency_of_established = true;

  Member *_new_member = nullptr;
  Inputs _inputs;

  PhaseOscillator _phase_oscillator;
  PhaseAnalyzer _phase_analyzer;

  Timeline _timeline;
  TimePosition _timeline_position;

  AntipopFilter _read_antipop_filter;
  AntipopFilter _write_antipop_filter;

  Options _options;

  void step();
  float readAll();
  float readEstablished();

  void setCircleToMember(unsigned int member_i);
  void skipToFocusedMember();

  bool isRecording();
  void forward();
  void backward();
  void forget();

  void toggleTuneToFrequencyOfEstablished();

  void toggleMemberMode();

  int getMostRecentLoopLength();
  float getPhaseOfEstablished();

  void nextSection();
  void deleteMember(unsigned int member_i);
  void nextGroup();

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
