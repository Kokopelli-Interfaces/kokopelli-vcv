#include "FrameEngine.hpp"

using namespace myrisa::dsp;

FrameEngine::FrameEngine() {
  for (int i = 0; i < numSections; i++) {
    _sections.push_back(new Section());
  }
}

void FrameEngine::handleModeChange() {
  if (_active_section) {
    printf("MODE CHANGE:: %d -> %d\n", _prev_mode, _mode);

    // FIXME BLOAT vv
    if (_active_section->_section_mode != RecordMode::READ && _mode == RecordMode::READ) {
      assert(_active_section->_active_layer != nullptr);
      if (_active_section->isEmpty() && !_active_section->_phase_defined) {
        _active_section->_phase_oscillator.setPitch(1 / (_active_section->_active_layer->samples_per_division * _sample_time));
        _active_section->_phase_defined = true;
        printf("_phase defined with pitch %f, s/div %d, s_time %f\n", _active_section->_phase_oscillator.freq, _active_section->_active_layer->samples_per_division, _sample_time);
        _active_section->_division_time_s = 1 / _active_section->_phase_oscillator.freq;
      }

      printf("END recording\n");
      printf("-- _section_mode %d, start div: %d, length: %d\n", _active_section->_section_mode,
             _active_section->_active_layer->start_division, _active_section->_active_layer->n_divisions);

      _active_section->_layers.push_back(_active_section->_active_layer);
      _active_section->_active_layer = nullptr;
    }

    if (_active_section->_section_mode == RecordMode::READ && _mode != RecordMode::READ) {
      addLayer(_active_section, _mode);
    }

    _active_section->_section_mode = _mode;

    // FIXME ^^ BLOAT

    _prev_mode = _mode;
  }
}

void FrameEngine::updateSectionPosition(float section_position) {
  _section_position = section_position;
  int active_section_i = round(section_position);
  if (active_section_i == numSections) {
    active_section_i--;
  }
  _active_section = _sections[active_section_i];
}

void FrameEngine::step() {
  if (_prev_mode != _mode) {
    handleModeChange();
  }

  for (auto section : _sections) {
    stepSection(section);
  }
}

float FrameEngine::read() {
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


void FrameEngine::stepSection(Section* section) {
  if (section == _active_section) {
    Section::Layer* _active_layer = section->_active_layer;
    if (_mode != RecordMode::READ) {
      assert(_active_layer != nullptr);
    }

    if (_mode == RecordMode::DUB && (_active_layer->start_division + _active_layer->n_divisions == section->_section_division)) {
      printf("END recording via overdub\n");
      printf("-- start div: %d, length: %d\n", _active_layer->start_division, _active_layer->n_divisions);
      section->_layers.push_back(_active_layer);

      // TODO FIXME depends on inputs
      section->_selected_layers = section->_layers;
      addLayer(section, RecordMode::DUB);
    }

    if (_mode != RecordMode::READ) {
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
  } else if (section->_phase_defined) {
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
      section->_section_division--;
    } else if (phase_change < 0) {
      section->_section_division++;
    }
  }

  // FIXME has a hard time with internal _section_division loops
  if (_use_ext_phase) {
    if (phase_flip && 0 < phase_change) {
      section->_freq_calculator_last_capture_phase_distance += phase_change - 1;
    } else if (phase_flip && phase_change < 0) {
      section->_freq_calculator_last_capture_phase_distance += phase_change + 1;
    } else {
      bool in_division_phase_flip =
          (0 < section->_freq_calculator_last_capture_phase_distance &&
           phase_change < 0) ||
          (section->_freq_calculator_last_capture_phase_distance < 0 &&
           0 < phase_change);

      if (!in_division_phase_flip) {
        section->_freq_calculator_last_capture_phase_distance += phase_change;
      }
    }

    if (section->_ext_phase_freq_calculator.process()) {
      float ext_phase_change = fabs(section->_freq_calculator_last_capture_phase_distance);
      float phase_change_per_sample = ext_phase_change / section->_ext_phase_freq_calculator.getDivision();
      if (phase_change_per_sample == 0) {
        return;
      }
      section->_division_time_s = _sample_time / phase_change_per_sample;
      section->_phase_defined = true;
      section->_freq_calculator_last_capture_phase_distance = 0;
    }
  }
}

void FrameEngine::addLayer(Section *section, RecordMode layer_mode) {
  // TODO FIXME
  section->_selected_layers = section->_layers;

  int samples_per_division = floor(section->_division_time_s / _sample_time);
  printf("NEW LAYER: _section_division: %d, div time s %f sample time %f sapls per %d _section_mode %d sel size %d\n", section->_section_division, section->_division_time_s, _sample_time, samples_per_division, layer_mode, section->_selected_layers.size());

  section->_active_layer = new Section::Layer(layer_mode, section->_section_division, section->_selected_layers, samples_per_division, section->_phase_defined);
}
