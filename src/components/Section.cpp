#include "Section.hpp"

using namespace myrisa;

inline void Section::finishNewLayer() {
  assert(new_layer != NULL);
  // FIXME  just using last layer as example

  printf("END recording\n");
  printf("-- start div: %f, length: %f\n", new_layer->start_division, new_layer->length);

}

// FIXME performance
inline float Section::getLayerAttenuation(int layer_i) {
  float layer_attenuation = 0.0f;
  if (new_layer) {
    for (auto target_layer : new_layer->target_layers) {
      if (target_layer == layers[layer_i]) {
        layer_attenuation += last_attenuation;
        break;
      }
    }
  }

  if (1.0f <= layer_attenuation) {
    return 1.0f;
  }

  for (unsigned int j = layer_i + 1; j < layers.size(); j++) {
    for (auto target_layer : layers[j]->target_layers) {
      if (target_layer == layers[layer_i]) {
        layer_attenuation += layers[j]->readSendAttenuation(position);
        if (1.0f <= layer_attenuation) {
          return 1.0f;
        }
      }
    }
  }

  return layer_attenuation;
}

float Section::read() {
  float out = 0.0f;
  for (unsigned int i = 0; i < layers.size(); i++) {
    float layer_attenuation = getLayerAttenuation(i);
    if (layer_attenuation < 1.0f) {
      float layer_out = layers[i]->readSampleWithAttenuation(division, phase, layer_attenuation);
      out += layer_out;
    }
  }

  return out;
}

inline void Section::advancePosition(float sample_time, bool use_ext_phase, float ext_phase) {
  last_sample_time = sample_time;
  last_phase = phase;
  if (use_ext_phase) {
    ASSERT(0, <=, ext_phase);
    ASSERT(ext_phase, <=, 1.0f);
    phase = ext_phase;
    phase_defined = true;
  } else if (!phase_defined) {
    phase = 0;
  } else {
    phase_oscillator.step(sample_time);
    phase = phase_oscillator.getPhase();
  }

  float phase_change = phase - last_phase;
  bool phase_flip = (fabsf(phase_change) > 0.95f && fabsf(phase_change) <= 1.0f);

  if (phase_flip) {
    if (0 < phase_change && 0 < division) {
      division--;
    } else if (phase_change < 0) {
      division++;
    }
  }
}

void Section::step(float in, float attenuation_power, float sample_time, bool use_ext_phase, float ext_phase) {
  last_attenuation = attenuation_power;

  if (mode != Mode::READ) {
    assert(new_layer != NULL);
  }

  if (mode == Mode::DUB && (new_layer->start_division + new_layer->n_divisions == division)) {
    printf("END recording via overdub\n");
    printf("-- start div: %f, length: %f\n", new_layer->start_division, new_layer->length);
    finishNewLayer();
    new_layer = new Layer(Mode::DUB, division, selected_layers);
  }

  if (mode != Mode::READ) {
    new_layer->write(division, phase, in, attenuation_power);
  }

  advancePosition(sample_time, use_ext_phase, ext_phase);
}


void Section::setMode(Mode new_mode) {
  if (mode != Mode::READ && new_mode == Mode::READ) {
    assert(new_layer != NULL);
    if (new_layer->mode == Mode::DEFINE_DIVISION_LENGTH) {
      phase_oscillator.setPitch(1 / (new_layer->samples_per_division * last_sample_time));
      phase_defined = true;
    }

    layers.push_back(new_layer);
    new_layer = NULL;
  }

  if (mode == Mode::READ && new_mode != Mode::READ) {
    startNewLayer(new_mode);
  }

  mode = new_mode;
}

bool Section::isEmpty() { return (layers.size() == 0); }
