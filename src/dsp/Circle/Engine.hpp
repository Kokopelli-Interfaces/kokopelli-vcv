#pragma once

#include "CircleGroup.hpp"
#include "definitions.hpp"
#include "dsp/PhaseAnalyzer.hpp"
#include "dsp/PhaseOscillator.hpp"
#include "dsp/AntipopFilter.hpp"
#include "dsp/Signal.hpp"
#include "rack.hpp"

#include <assert.h>
#include <math.h>
#include <vector>
#include <numeric> // std::iota

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct Engine {
  // externally set
  bool _use_ext_phase = false;
  float _ext_phase = 0.f;
  float _sample_time = 1.0f;
  // TODO make me an array to support MIX4 & PLAY
  kokopellivcv::dsp::SignalType _signal_type;
  // ---

  CircleGroup *_main_group;
  CircleGroup *_focused_group;

  // for progressing the circle with an internal phase
  PhaseOscillator _phase_oscillator;
  PhaseAnalyzer _phase_analyzer;

  std::vector<unsigned int> _selected_members_idx;
  std::vector<unsigned int> _saved_selected_members_idx;
  bool _new_member_active = true;
  bool _select_new_members = true;

  unsigned int _active_member_i;

  CircleMember *_recording_member = nullptr;
  RecordParams _record_params;

  TimePosition _timeline_position;

  LoopMode _loop_mode = LoopMode::Group;

  AntipopFilter _read_antipop_filter;
  AntipopFilter _write_antipop_filter;

  Options _options;

  void loop();
  void loopLongPress();
  void next();
  void prev();

  void step();
  float read();
  float readSelection();
  float readActiveMember();

  void skipToActiveMember();

  void undo();
  bool isRecording();
  void resetEngineMode();

  bool checkState(int skip_back, int fix_bounds, int record_on_inner_circle);

  void selectRange(unsigned int member_i_1, unsigned int member_i_2);
  void soloSelectMember(unsigned int member_i);
  bool isSelected(unsigned int member_i);
  bool isSolo(unsigned int member_i);
  void toggleSelectMember(unsigned int member_i);
  void deleteMember(unsigned int member_i);
  void deleteSelection();

private:
  void setFixBounds(bool fix_bounds);
  void setRecordOnInnerLoop(bool record_on_inner_circle);

  // unsigned int updateCirclePeriod();

  void endRecording(bool loop_recording);
  CircleMember* newRecording();
  inline bool phaseDefined();
  inline void write();
  inline void handleBeatChange(PhaseAnalyzer::PhaseEvent flip);
  inline PhaseAnalyzer::PhaseEvent advanceTimelinePosition();
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
