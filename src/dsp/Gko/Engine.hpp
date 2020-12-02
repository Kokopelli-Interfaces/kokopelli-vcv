#pragma once

#include "Timeline.hpp"
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

  /* read only */

  Layer *_recording = nullptr;
  bool _recording_active = false;
  RecordParams _record_params;

  PhaseOscillator _phase_oscillator;
  PhaseAnalyzer _phase_analyzer;

  Timeline _timeline;
  TimelinePosition _timeline_position;
  TimeFrame _read_time_frame;

  void step();
  float read();

private:
  inline void write();
  inline void endRecording();
  inline void beginRecording();
  inline void handlePhaseFlip(PhaseAnalyzer::PhaseFlip flip);
  inline PhaseAnalyzer::PhaseFlip advanceTimelinePosition();
};

} // namespace frame
} // namespace dsp
} // namespace myrisa
