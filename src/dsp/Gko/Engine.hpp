#pragma once

#include "Timeline.hpp"
#include "utils.hpp"
#include "definitions.hpp"
#include "dsp/PhaseAnalyzer.hpp"
#include "dsp/PhaseOscillator.hpp"
#include "Layer.hpp"
#include "rack.hpp"

#include <assert.h>
#include <math.h>
#include <vector>

namespace myrisa {
namespace dsp {
namespace gko {

struct Engine {
  bool _use_ext_phase = false;
  float _ext_phase = 0.f;
  float _sample_time = 1.0f;
  std::vector<int> _selected_layers;

  /* read only */

  RecordParams _record;
  Layer *_recording = nullptr;

  PhaseOscillator _phase_oscillator;
  PhaseAnalyzer _phase_analyzer;

  TimeFrame _time_frame;

  Timeline _timeline;
  float _time = 0.f;

  const float _recordActiveThreshold = 0.0001f;

  void step();
  float read();

  void setRecordMode(RecordParams::Mode mode);
  void setRecordTimeFrame(TimeFrame time_frame);
  void setRecordStrength(float strength);

private:
  void record();
  void beginRecording();
  void endRecording();
  inline PhaseAnalyzer::PhaseFlip advanceTimelinePosition();
  inline void handlePhaseFlip();
};

} // namespace frame
} // namespace dsp
} // namespace myrisa
