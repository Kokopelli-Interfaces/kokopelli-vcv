#pragma once

#include "Scene.hpp"
#include "definitions.hpp"
#include "dsp/PhaseAnalyzer.hpp"
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
  float _ext_phase = 0.0f;
  bool _use_ext_phase = false;

  // TODO vector
  float _in = 0.0f;
  float _sample_time = 1.0f;
  float _scene_position = 0.0f;

  /* read only */

  Layer *_new_layer = nullptr;

  PhaseOscillator _phase_oscillator;

  TimeFrame _time_frame;

  // TODO make me infinite
  const int _numScenes = 16;
  Delta _delta = Delta();

  Scene *recording_dest_scene = nullptr;
  std::vector<Scene*> _scenes;
  PhaseAnalyzer _phase_analyzer;

private:
  const float _deltaActivePowerThreshold = 0.0001f;

public:
  Engine();
  ~Engine();
  void updateScenePosition(float scene_position);
  void step();
  float read();

  void updateDeltaMode(Delta::Mode mode);
  void updateDeltaContext(Delta::Context mode);
  void updateDeltaPower(float power);

private:
  void stepScene(Scene *scene);
  bool addLayer(Scene *scene);
};

} // namespace frame
} // namespace dsp
} // namespace myrisa
