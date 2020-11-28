#pragma once

#include "Scene.hpp"
#include "definitions.hpp"
#include "dsp/PhaseAnalyzer.hpp"
#include "Layer.hpp"
#include "rack.hpp"
#include "util/assert.hpp"
#include <assert.h>
#include <math.h>
#include <vector>

namespace myrisa {
namespace dsp {
namespace frame {

class Engine {
public:
  bool _use_ext_phase = false;
  float _ext_phase = 0.0f;
  float _in = 0.0f;
  float _sample_time = 1.0f;
  float _scene_position = 0.0f;
  float _attenuation = 0.0f;
  Delta::Mode _mode = Delta::Mode::READ;
  Scene *_active_scene = nullptr;

  // all scenes have the same phase. the way users can create new scenes of different bpm, is to use
  // another Frame to automate the rate of the external phase and section
  float _phase = 0.f;

  TimeFrame time_frame_mode;

  Delta _delta = Delta();

private:
  const int numScenes = 16;
  Scene *recording_dest_scene = nullptr;
  std::vector<Scene*> _scenes;
  Delta::Mode _active_mode = Delta::Mode::READ;

  PhaseAnalyzer _phase_analyzer;

public:
  Engine();
  ~Engine();
  void updateScenePosition(float scene_position);
  void step();
  float read();

private:
  void handleModeChange();
  void stepScene(Scene *scene);
  bool addLayer(Scene *scene, Delta::Mode mode);
};

} // namespace frame
} // namespace dsp
} // namespace myrisa
