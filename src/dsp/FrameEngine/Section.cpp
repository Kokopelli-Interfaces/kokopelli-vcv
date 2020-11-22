#include "FrameEngine.hpp"

using namespace myrisa::dsp;

FrameEngine::Section::Section(FrameEngine *engine) {
  _engine = engine;
  _ext_phase_freq_calculator.setDivision(20000);
}

FrameEngine::Section::~Section() {
  for (auto layer : _layers) {
    delete layer;
  }

  if (_active_layer) {
    delete _active_layer;
  }
}

// FIXME performance
float FrameEngine::Section::getLayerAttenuation(int layer_i) {
  float layer_attenuation = 0.0f;
  if (_active_layer) {
    for (auto target_layer : _active_layer->target_layers) {
      if (target_layer == _layers[layer_i]) {
        layer_attenuation += _engine->_attenuation;
        break;
      }
    }
  }

  if (1.0f <= layer_attenuation) {
    return 1.0f;
  }

  for (unsigned int j = layer_i + 1; j < _layers.size(); j++) {
    for (auto target_layer : _layers[j]->target_layers) {
      if (target_layer == _layers[layer_i]) {
        layer_attenuation += _layers[j]->readSendAttenuation(_section_division, _phase);
        if (1.0f <= layer_attenuation) {
          return 1.0f;
        }
      }
    }
  }

  return layer_attenuation;
}

float FrameEngine::Section::read() {
  float out = 0.0f;
  for (unsigned int i = 0; i < _layers.size(); i++) {
    float layer_attenuation = getLayerAttenuation(i);
    if (layer_attenuation < 1.0f) {
      float layer_out = _layers[i]->readSampleWithAttenuation(_section_division, _phase, layer_attenuation);
      out += layer_out;
    }
  }

  return out;
}

void FrameEngine::Section::advance() {
  float prev_phase = _phase;

  if (_engine->_use_ext_phase) {
    ASSERT(0, <=, _engine->_ext_phase);
    ASSERT(_engine->_ext_phase, <=, 1.0f);
    _phase = _engine->_ext_phase;
  } else if (_phase_defined) {
    _phase_oscillator.step(_engine->_sample_time);
    _phase = _phase_oscillator.getPhase();
  } else {
    _phase = 0;
  }

  float phase_change = _phase - prev_phase;
  float phase_abs_change = fabs(phase_change);
  bool phase_flip = (phase_abs_change > 0.95 && phase_abs_change <= 1.0);

  if (phase_flip) {
    if (0 < phase_change && 0 < _section_division) {
      _section_division--;
    } else if (phase_change < 0) {
      _section_division++;
    }
  }

  // FIXME has a hard time with internal _section_division loops
  if (_engine->_use_ext_phase) {
    if (phase_flip && 0 < phase_change) {
      _freq_calculator_last_capture_phase_distance += phase_change - 1;
    } else if (phase_flip && phase_change < 0) {
      _freq_calculator_last_capture_phase_distance += phase_change + 1;
    } else {
      bool in_division_phase_flip =
          (0 < _freq_calculator_last_capture_phase_distance &&
           phase_change < 0) ||
          (_freq_calculator_last_capture_phase_distance < 0 &&
           0 < phase_change);

      if (!in_division_phase_flip) {
        _freq_calculator_last_capture_phase_distance += phase_change;
      }
    }

    if (_ext_phase_freq_calculator.process()) {
      float ext_phase_change = fabs(_freq_calculator_last_capture_phase_distance);
      float phase_change_per_sample = ext_phase_change / _ext_phase_freq_calculator.getDivision();
      if (phase_change_per_sample == 0) {
        return;
      }
      _division_time_s = _engine->_sample_time / phase_change_per_sample;
      _phase_defined = true;
      _freq_calculator_last_capture_phase_distance = 0;
    }
  }
}

void FrameEngine::Section::newLayer(RecordMode layer_mode) {
  if (layer_mode != RecordMode::DEFINE_DIVISION_LENGTH && !_phase_defined) {
    // TODO
    printf("Myrisa Frame Error: New layer that isn't define _section_division, but no _phase defined.\n");
    return;
  }

  // TODO FIXME
  _selected_layers = _layers;

  int samples_per_division = floor(_division_time_s / _engine->_sample_time);
  printf("NEW LAYER: _section_division: %d, div time s %f sample time %f sapls per %d _mode %d sel size %d\n", _section_division, _division_time_s, _engine->_sample_time, samples_per_division, layer_mode, _selected_layers.size());

  _active_layer = new Layer(layer_mode, _section_division, _selected_layers, samples_per_division);
}

void FrameEngine::Section::step() {
  if (_mode != RecordMode::READ) {
    assert(_active_layer != nullptr);
  }

  if (_mode == RecordMode::DUB && (_active_layer->start_division + _active_layer->n_divisions == _section_division)) {
    printf("END recording via overdub\n");
    printf("-- start div: %d, length: %d\n", _active_layer->start_division, _active_layer->n_divisions);
    _layers.push_back(_active_layer);

    // TODO FIXME
    _selected_layers = _layers;
    newLayer(RecordMode::DUB);
  }

  if (_mode != RecordMode::READ) {
    _active_layer->write(_section_division, _phase, _engine->_in, _engine->_attenuation);
  }

  advance();
}

void FrameEngine::Section::setRecordMode(RecordMode new_mode) {
  if (_mode != RecordMode::READ && new_mode == RecordMode::READ) {
    assert(_active_layer != nullptr);
    if (_active_layer->_mode == RecordMode::DEFINE_DIVISION_LENGTH) {
      _phase_oscillator.setPitch(1 / (_active_layer->samples_per_division * _engine->_sample_time));
      _phase_defined = true;
      printf("_phase defined with pitch %f, s/div %d, s_time %f\n", _phase_oscillator.freq, _active_layer->samples_per_division, _engine->_sample_time);
      _division_time_s = 1 / _phase_oscillator.freq;
    }

    printf("END recording\n");
    printf("-- _mode %d, start div: %d, length: %d\n", _active_layer->_mode,
           _active_layer->start_division, _active_layer->n_divisions);

    _layers.push_back(_active_layer);
    _active_layer = nullptr;
  }

  if (_mode == RecordMode::READ && new_mode != RecordMode::READ) {
    newLayer(new_mode);
  }

  _mode = new_mode;
}

bool FrameEngine::Section::isEmpty() { return (_layers.size() == 0); }
