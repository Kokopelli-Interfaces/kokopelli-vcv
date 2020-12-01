#include "Engine.hpp"

using namespace myrisa::dsp::gko;

inline void Engine::endManifestation() {
    assert(_manifestation != nullptr);
    assert(_manifestation->length != 0.f);
    assert(_manifestation->signal->size() != 0);

    float recording_time = _manifestation->signal->size() * _sample_time;

    printf("Manifest De-Activate\n");
    printf("-- Manifestation start %f length %f size %d recording time %fs loop %d samples_per_beat %d\n", _manifestation->start, _manifestation->length, _manifestation->signal->size(), recording_time, _manifestation->loop, _manifestation->samples_per_beat);

    if (!_phase_oscillator.isSet()) {
      if (_use_ext_phase && _phase_analyzer.getDivisionPeriod() != 0) {
        _phase_oscillator.setFrequency(1 / _phase_analyzer.getDivisionPeriod());
      } else {
        _phase_oscillator.setFrequency(1 / recording_time);
      }
      printf("-- phase oscillator set with frequency: %f\n", _phase_oscillator.getFrequency());
    }

    _timeline.layers.push_back(_manifestation);

    _manifestation = nullptr;
}

void Engine::beginManifestation() {
  assert(_manifest.active);
  assert(_manifestation == nullptr);

  float length = 1.f;
  if (_manifest.mode == Manifest::Mode::DUB && 0 < _manifest.selected_layers.size()) {
    length = _timeline.getLengthOfLayers(_manifest.selected_layers);
  }

  float start_time;
  if (_manifest.mode == Manifest::Mode::EXTEND) {
    start_time = std::round(_time);
  } else {
    start_time = std::floor(_time);
  }

  _manifestation = new Layer(start_time, length, _manifest.selected_layers);

  bool phase_defined = (_use_ext_phase || _phase_oscillator.isSet());
  if (phase_defined) {
    float phase_period;
    if (_use_ext_phase) {
      phase_period = _phase_analyzer.getDivisionPeriod();
    } else if (_phase_oscillator.isSet()) {
      phase_period = 1 / _phase_oscillator.getFrequency();
    }
    _manifestation->samples_per_beat = floor(phase_period / _sample_time);
    _manifestation->resizeToLength();
  }

  if (_manifest.time_frame != TimeFrame::TIMELINE) {
    _manifestation->loop = true;
  }

  printf("-- Manifestation start %f initial length %f loop %d samples per beat %d\n", _manifestation->start, _manifestation->length, _manifestation->loop, _manifestation->samples_per_beat);
}

void Engine::setManifestStrength(float strength) {
  _manifest.strength = strength;

  bool manifest_activate = !_manifest.active && _manifestActiveThreshold <= strength;
  bool manifest_deactivate = _manifest.active && strength < _manifestActiveThreshold ;

  if (manifest_activate) {
    printf("Manifest Activate\n");
    _manifest.active = true;
    _manifest.selected_layers = _selected_layers;
    beginManifestation();
  } else if (manifest_deactivate) {
    _manifest.active = false;
    endManifestation();
  }
}

void Engine::setManifestTimeFrame(TimeFrame time_frame) {
  if (_manifest.active) {
    assert(_manifestation != nullptr);

    if (time_frame != TimeFrame::TIMELINE) {
      _manifestation->loop = true;
    }
  }

  _manifest.time_frame = time_frame;
}

void Engine::setManifestMode(Manifest::Mode mode) {
  if (mode == _manifest.mode) {
    return;
  }

  // TODO transitions
  _manifest.mode = mode;
}
