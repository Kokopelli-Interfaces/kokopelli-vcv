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
namespace frame {

class Engine {
public:
  bool _use_ext_phase = false;
  float _ext_phase = 0.f;

  float _sample_time = 1.0f;

  /* read only */

  struct Time {
    float phase = 0.f;
    int beat = 0;
  };
  Time _time;

  struct Manifestation {
    Layer *content = nullptr;
    std::vector<int> target_layers;
    Time start;
  };
  Manifestation _active_manifestation;

  PhaseOscillator _phase_oscillator;
  PhaseAnalyzer _phase_analyzer;

  ManifestParams _manifest;
  TimeFrame _time_frame;
  Timeline *_timeline;

  // TODO
  std::vector<int> _selected_layers;

  // TODO make me infinite
  const float _manifestActiveThreshold = 0.0001f;

public:
  Engine();
  ~Engine();
  void setScenePosition(float scene_position);
  void step();
  float read();

  void setManifestMode(ManifestParams::Mode mode);
  void setManifestTimeFrame(TimeFrame mode);
  void setManifestStrength(float strength);
};

} // namespace frame
} // namespace dsp
} // namespace myrisa
