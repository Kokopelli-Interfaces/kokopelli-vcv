#include "Engine.hpp"

using namespace myrisa::dsp::frame;

Engine::Engine() {
  for (int i = 0; i < _numScenes; i++) {
    _scenes.push_back(new Scene());
  }
}

Engine::~Engine() {
  for (int i = 0; i < _numScenes; i++) {
    delete _scenes[i];
  }
}

void Engine::setScenePosition(float scene_position) {
  _scene_position = scene_position;
  int active_scene_i = std::round(scene_position);
  if (active_scene_i == _numScenes) {
    active_scene_i--;
  }
  _active_scene = _scenes[active_scene_i];
}

inline Manifestation* createManifestation() {
    _manifest.active = true;
    _active_manifestation = Manifestation();
    _active_manifestation.content = new Layer();
    _active_manifestation.start = _time;
    _active_manifestation.target_layers = _seelcted_layers;
}

void Engine::initializePhaseOscillator() {
  assert(!_phase_oscillator.isSet())
  if (_use_ext_phase) {
    assert(_phase_analyzer.getDivisionPeriod() != 0);
    _phase_oscillator.setFrequency(1 / _phase_analyzer.getDivisionPeriod());
  } else {
    assert(_active_manifestation.start.beat == 0 && _active_manifestation.start.phase == 0);
    assert(_time.phase == 0 && _time.beat == 0);
    float manifestation_period = _active_manifestation.buffer.size() * _sample_time;
    _phase_oscillator.setFrequency(1 / manifestation_period);
  }
}

void Engine::setManifestStrength(float strength) {
  _manifest.strength = strength;

  bool manifest_activate = !_manifest.active && _manifestActiveThreshold <= strength;
  bool manifest_deactivate = _manifest.active && strength < _manifestActiveThreshold ;

  if (manifest_activate) {
    assert(_active_manifestation.content == nullptr);

    printf("Manifest Activate\n");
    _active_manifestation = createManifestation();
    if (!_timeline->sectionExistsForBeat(_time.beat)) {
      // TODO make me start as a copy
      Section* new_section = new Section();
      _timeline->pushBackSection(new_section);
    }
  } else if (manifest_deactivate) {
    assert(_active_manifestation.content != nullptr);
    assert(_active_manifestation.content->n_beats != 0);
    assert(_active_manifestation.content->buffer.size() != 0);

    printf("Manifest De-Activate\n");
    if (!_phase_oscillator.isSet()) {
      initializePhaseOscillator();
    }

    int start_beat = _active_manifestation.start_beat;
    Section* manifestation_section = _timeline->getSectionAndSectionBeatAtBeat(start_beat).first;
    assert(manifestation_section != nullptr);
    manifestation_section->layers.push_back(_active_manifestation.content);
    // TODO how to do TIME mode? have to alter all layers.
    manifestation_section->layer_targets.push_back(_active_manifestation.content);

  }
}

void Engine::setManifestMode(ManifestParams::Mode mode) {
  if (mode == _manifest.mode) {
    return;
  }

  // TODO transitions
  _manifest.mode = mode;
}

void Engine::advancePosition() {
  if (_use_ext_phase) {
    _time.phase = _ext_phase;
  } else {
    _time.phase = _phase_oscillator.step(_sample_time);
  }
  assert(0 <= _time.phase);
  assert(_time.phase <= 1.0f);

  PhaseAnalyzer::PhaseFlip phase_flip = _phase_analyzer.process(_time.phase, _sample_time);

  // TODO use _time_frame
  if (phase_flip == PhaseAnalyzer::PhaseFlip::BACKWARD && 0 < _time.beat) {
    _time.beat--;
  } else if (phase_flip == PhaseAnalyzer::FORWARD) {
    _time.beat++;
  }
}

void Engine::manifest() {
  assert(_manifest.active);
}

inline void appendTimelineSection(Timeline *timeline) {
  timeline->sections.push_back(new Section());
}

void Engine::stepScene(Scene* scene) {
  if (scene == _active_scene) {
    Layer* _active_layer = scene->_active_layer;
    if (_manifest.active) {
      assert(_active_layer != nullptr);
    }

    if (_manifest.mode == ManifestParams::Mode::DUB && (_active_layer->start_division + _active_layer->n_divisions == scene->_scene_division)) {
      printf("END recording via overdub\n");
      printf("-- start div: %d, length: %d\n", _active_layer->start_division, _active_layer->n_divisions);
      scene->_layers.push_back(_active_layer);

      // TODO FIXME depends on inputs
      scene->_selected_layers = scene->_layers;
      addLayer(scene);
    }

    if (_manifest.active) {
      _active_layer->write(scene->_scene_division, scene->_time.phase, _manifest.in, _manifest.strength);
    }
  }


}

void stepSection() {

  float out = 0.f;
  for (auto layer : layers) {
    float layer_out = _layers[i]->readSampleWithAttenuation(_scene_division, _time.phase, layer_attenuation);
    out += layer_out;
  }

}

bool Engine::addLayer() {
  // TODO FIXME
  scene->_selected_layers = scene->_layers;

  float phase_period = 0.0;
  if (_use_ext_phase) {
    phase_period = _phase_analyzer._phase_period_estimate;
    // indicates infinity, signal is not moving, don't add layer
    if (phase_period == -1) {
      return false;
    }
  } else if (scene->_internal_phase_defined) {
    phase_period = 1 / scene->_phase_oscillator.freq;
  }

  int samples_per_division = floor(phase_period / _sample_time);

  printf("NEW LAYER: _scene_division: %d, phase period s %f sample time %f sapls per %d mode %d sel size %ld\n", scene->_scene_division, phase_period, _sample_time, samples_per_division, _manifest.mode, scene->_selected_layers.size());

  bool phase_def = _use_ext_phase ? true : scene->_internal_phase_defined;

  scene->_active_layer = new Layer(_manifest.mode, scene->_scene_division, scene->_selected_layers, samples_per_division, phase_def);

  return true;
}

float Engine::read() {
  assert(_time.beat < _timeline->beat_to_section_i.size());
  assert(_timeline->sections.size() == _timeline->sections_info.size());

  int i = _timeline->beat_to_section_i[time_b];
  Section *current_section = _timeline->sections[i];
  Timeline::SectionInfo current_section_info = _timeline->sections_info[i];

  assert(_time.beat < current_section_data.section_end);
  assert(current_section_data.section_start <= _time.beat);

  int section_beat = _time.beat - current_section_data.section_start;

  // TODO also read layers in TIME frame, and take into consideration Takes, mix and other things...
  return current_section->render->read(section_beat + _time.phase);
}
