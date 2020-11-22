#include "Engine.hpp"

using namespace myrisa::dsp::frame;

Engine::Engine() {
  for (int i = 0; i < numSections; i++) {
    _sections.push_back(new Section());
  }
}

void Engine::handleModeChange() {
  if (_active_section) {
    printf("MODE CHANGE:: %d -> %d\n", _active_mode, _mode);

    // FIXME BLOAT vv
    if (_active_mode != RecordMode::READ && _mode == RecordMode::READ) {
      assert(_active_section->_active_layer != nullptr);

      if (_active_section->isEmpty() && !_active_section->_internal_phase_defined) {

        float division_period = 0;
        if (_use_ext_phase) {
          division_period = _phase_analyzer._phase_period_estimate;
        } else {
          division_period = _active_section->_active_layer->samples_per_division * _sample_time;
        }

        ASSERT(0.0, <, division_period);
        _active_section->_phase_oscillator.setPitch(1 / division_period);

        _active_section->_internal_phase_defined = true;
        printf("_phase defined with pitch %f, s/div %d, s_time %f\n", _active_section->_phase_oscillator.freq, _active_section->_active_layer->samples_per_division, _sample_time);
      }

      printf("END recording\n");
      printf("-- mode %d, start div: %d, length: %d\n", _mode,
             _active_section->_active_layer->start_division, _active_section->_active_layer->n_divisions);

      _active_section->_layers.push_back(_active_section->_active_layer);
      _active_section->_active_layer = nullptr;
    }

    if (_active_mode == RecordMode::READ && _mode != RecordMode::READ) {
      if (addLayer(_active_section, _mode)) {
        _active_mode = _mode;
      }
    } else {
      _active_mode = _mode;
    }

    // FIXME ^^ BLOAT

  }
}

void Engine::updateSectionPosition(float section_position) {
  _section_position = section_position;
  int active_section_i = round(section_position);
  if (active_section_i == numSections) {
    active_section_i--;
  }
  _active_section = _sections[active_section_i];
}

void Engine::step() {
  if (_active_mode != _mode) {
    handleModeChange();
  }

  if (_use_ext_phase) {
    _phase_analyzer.process(_ext_phase, _sample_time);
  }

  for (auto section : _sections) {
    stepSection(section);
  }
}

float Engine::read() {
  int section_1 = floor(_section_position);
  int section_2 = ceil(_section_position);
  float weight = _section_position - floor(_section_position);

  float out = 0.0f;
  out += _sections[section_1]->read(_attenuation) * (1 - weight);
  if (section_1 != section_2 && section_2 < numSections) {
    out += _sections[section_2]->read(_attenuation) * weight;
  }

  return out;
}


void Engine::stepSection(Section* section) {
  if (section == _active_section) {
    Layer* _active_layer = section->_active_layer;
    if (_active_mode != RecordMode::READ) {
      assert(_active_layer != nullptr);
    }

    if (_active_mode == RecordMode::DUB && (_active_layer->start_division + _active_layer->n_divisions == section->_section_division)) {
      printf("END recording via overdub\n");
      printf("-- start div: %d, length: %d\n", _active_layer->start_division, _active_layer->n_divisions);
      section->_layers.push_back(_active_layer);

      // TODO FIXME depends on inputs
      section->_selected_layers = section->_layers;
      addLayer(section, _active_mode);
    }

    if (_active_mode != RecordMode::READ) {
      _active_layer->write(section->_section_division, section->_phase, _in, _attenuation);
    }
  }

  float prev_phase = section->_phase;

  // ADVANCE
  // TODO don't need to calculate for all sections
  // TODO should all sections have the same phase?
  if (_use_ext_phase) {
    ASSERT(0, <=, _ext_phase);
    ASSERT(_ext_phase, <=, 1.0f);
    section->_phase = _ext_phase;
  } else if (section->_internal_phase_defined) {
    section->_phase_oscillator.step(_sample_time);
    section->_phase = section->_phase_oscillator.getPhase();
  } else {
    section->_phase = 0;
  }

  float phase_change = section->_phase - prev_phase;
  float phase_abs_change = fabs(phase_change);
  bool phase_flip = (phase_abs_change > 0.95 && phase_abs_change <= 1.0);

  if (phase_flip) {
    if (0 < phase_change && 0 < section->_section_division) {
      // TODO loop back to end if in loop mode
      section->_section_division--;
    } else if (phase_change < 0) {
      section->_section_division++;
    }
  }
}

bool Engine::addLayer(Section *section, RecordMode layer_mode) {
  // TODO FIXME
  section->_selected_layers = section->_layers;

  float phase_period = 0.0;
  if (_use_ext_phase) {
    phase_period = _phase_analyzer._phase_period_estimate;
    // indicates infinity, signal is not moving, don't add layer
    if (phase_period == -1) {
      return false;
    }
  } else if (section->_internal_phase_defined) {
    phase_period = 1 / section->_phase_oscillator.freq;
  }

  int samples_per_division = floor(phase_period / _sample_time);

  printf("NEW LAYER: _section_division: %d, phase period s %f sample time %f sapls per %d mode %d sel size %d\n", section->_section_division, phase_period, _sample_time, samples_per_division, layer_mode, section->_selected_layers.size());

  bool phase_def = _use_ext_phase ? true : section->_internal_phase_defined;

  section->_active_layer = new Layer(layer_mode, section->_section_division, section->_selected_layers, samples_per_division, phase_def);

  return true;
}
