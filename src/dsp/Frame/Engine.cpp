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

void Engine::updateScenePosition(float scene_position) {
  _scene_position = scene_position;
  int active_scene_i = round(scene_position);
  if (active_scene_i == _numScenes) {
    active_scene_i--;
  }
  _active_scene = _scenes[active_scene_i];
}

void Engine::updateDeltaPower(float power) {
  _delta.power = power;

  bool delta_activate = !_delta.active && _deltaActivePowerThreshold <= power;
  bool delta_deactivate = _delta.active && power < _deltaActivePowerThreshold ;

  if (delta_activate) {
    if (addLayer(_active_scene)) {
      _delta.active = true;
    }
  } else if (delta_deactivate) {
    assert(_active_scene->_active_layer != nullptr);

    printf("END change\n");

    if (_active_scene->isEmpty() && !_active_scene->_internal_phase_defined) {
      float division_period = 0;
      if (_use_ext_phase) {
        division_period = _phase_analyzer._phase_period_estimate;
      } else {
        division_period = _active_scene->_active_layer->samples_per_division * _sample_time;
      }

      assert(0.0 < division_period);
      _active_scene->_phase_oscillator.setPitch(1 / division_period);

      _active_scene->_internal_phase_defined = true;
      printf("_phase defined with pitch %f, s/div %d, s_time %f\n", _active_scene->_phase_oscillator.freq, _active_scene->_active_layer->samples_per_division, _sample_time);
    }

    printf("-- mode %d, start div: %d, length: %d\n", _delta.mode, _active_scene->_active_layer->start_division, _active_scene->_active_layer->n_divisions);

    _active_scene->_layers.push_back(_active_scene->_active_layer);
    _active_scene->_active_layer = nullptr;
  }
}

void Engine::updateDeltaMode(Delta::Mode mode) {
  if (mode == _delta.mode) {
    return;
  }

  if (_active_scene) {
    printf("MODE CHANGE:: %d -> %d\n", _delta.mode, mode);

    // FIXME BLOAT vv
    // FIXME ^^ BLOAT
  }

}

void Engine::step() {
  // FIXME hack to work with old method
  if (_use_ext_phase) {
    _phase_analyzer.process(_ext_phase, _sample_time);
  }

  for (auto scene : _scenes) {
    stepScene(scene);
  }
}

float Engine::read() {
  int scene_1 = floor(_scene_position);
  int scene_2 = ceil(_scene_position);
  float weight = _scene_position - floor(_scene_position);

  float out = 0.0f;
  out += _scenes[scene_1]->read(_delta.power) * (1 - weight);
  if (scene_1 != scene_2 && scene_2 < _numScenes) {
    out += _scenes[scene_2]->read(_delta.power) * weight;
  }

  return out;
}


void Engine::stepScene(Scene* scene) {
  if (scene == _active_scene) {
    Layer* _active_layer = scene->_active_layer;
    if (_delta.active) {
      assert(_active_layer != nullptr);
    }

    if (_delta.mode == Delta::Mode::DUB && (_active_layer->start_division + _active_layer->n_divisions == scene->_scene_division)) {
      printf("END recording via overdub\n");
      printf("-- start div: %d, length: %d\n", _active_layer->start_division, _active_layer->n_divisions);
      scene->_layers.push_back(_active_layer);

      // TODO FIXME depends on inputs
      scene->_selected_layers = scene->_layers;
      addLayer(scene);
    }

    if (_delta.active) {
      _active_layer->write(scene->_scene_division, scene->_phase, _in, _delta.power);
    }
  }

  float prev_phase = scene->_phase;

  // ADVANCE
  // TODO don't need to calculate for all scenes
  // TODO should all scenes have the same phase?
  if (_use_ext_phase) {
    assert(0 <= _ext_phase);
    assert(_ext_phase <= 1.0f);
    scene->_phase = _ext_phase;
  } else if (scene->_internal_phase_defined) {
    scene->_phase_oscillator.step(_sample_time);
    scene->_phase = scene->_phase_oscillator.getPhase();
  } else {
    scene->_phase = 0;
  }

  float phase_change = scene->_phase - prev_phase;
  float phase_abs_change = fabs(phase_change);
  bool phase_flip = (phase_abs_change > 0.95 && phase_abs_change <= 1.0);

  if (phase_flip) {
    if (0 < phase_change && 0 < scene->_scene_division) {
      // TODO loop back to end if in loop mode
      scene->_scene_division--;
    } else if (phase_change < 0) {
      scene->_scene_division++;
    }
  }
}

bool Engine::addLayer(Scene *scene) {
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

  printf("NEW LAYER: _scene_division: %d, phase period s %f sample time %f sapls per %d mode %d sel size %ld\n", scene->_scene_division, phase_period, _sample_time, samples_per_division, _delta.mode, scene->_selected_layers.size());

  bool phase_def = _use_ext_phase ? true : scene->_internal_phase_defined;

  scene->_active_layer = new Layer(_delta.mode, scene->_scene_division, scene->_selected_layers, samples_per_division, phase_def);

  return true;
}
