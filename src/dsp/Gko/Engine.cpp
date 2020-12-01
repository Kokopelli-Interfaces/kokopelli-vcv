#include "Engine.hpp"

using namespace myrisa::dsp::gko;

inline void initializePhaseOscillator(myrisa::dsp::PhaseOscillator oscillator, myrisa::dsp::PhaseAnalyzer analyzer, bool use_ext_phase, int last_recording_length) {
  assert(!oscillator.isSet());
  if (use_ext_phase && analyzer.getDivisionPeriod() != 0) {
    oscillator.setFrequency(1 / analyzer.getDivisionPeriod());
  } else {
    oscillator.setFrequency(1 / last_recording_length);
  }
}

inline void Engine::endManifestation() {
    assert(_manifestation != nullptr);
    assert(_manifestation->length != 0.f);
    assert(_manifestation->signal->size() != 0);

    if (!_phase_oscillator.isSet()) {
      int recording_length = _manifestation->signal->size() * _sample_time;
      initializePhaseOscillator(_phase_oscillator, _phase_analyzer, _use_ext_phase, recording_length);
    }

    _timeline.layers.push_back(_manifestation);

    _manifestation = nullptr;
}

void Engine::beginManifestation() {
  assert(_manifest.active);
  assert(_manifestation == nullptr);

  float length = 0.f;
  if (_manifest.mode == Manifest::Mode::DUB) {
    if (0 < _manifest.selected_layers.size()) {
      length = _timeline.getLengthOfLayers(_manifest.selected_layers);
    } else {
      length = 1.f;
    }
  }

  _manifestation = new Layer(_time, length, _manifest.selected_layers);

  bool phase_defined = (_use_ext_phase || _phase_oscillator.isSet());
  if (phase_defined) {
    float phase_period;
    if (_use_ext_phase) {
      phase_period = _phase_analyzer.getDivisionPeriod();
    } else if (_phase_oscillator.isSet()) {
      phase_period = 1 / _phase_oscillator.getFrequency();
    }
    int samples_per_beat = floor(phase_period / _sample_time);
    _manifestation->signal->resize(samples_per_beat * length);
    _manifestation->manifestation_strength->resize(samples_per_beat * length);
  }

  if (_manifest.time_frame != TimeFrame::TIMELINE) {
    _manifestation->loop = true;
  }
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
    printf("Manifest De-Activate\n");
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
