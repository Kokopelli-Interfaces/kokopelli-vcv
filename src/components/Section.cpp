#include "Section.hpp"

using namespace myrisa;

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
        layer_attenuation += layers[j]->readSendAttenuation(division, phase);
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

inline void Section::advance(float sample_time, bool use_ext_phase, float ext_phase) {
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

  if (mode != RecordMode::READ) {
    assert(new_layer != NULL);
  }

  if (mode == RecordMode::DUB && (new_layer->start_division + new_layer->n_divisions == division)) {
    printf("END recording via overdub\n");
    printf("-- start div: %d, length: %d\n", new_layer->start_division, new_layer->n_divisions);
    layers.push_back(new_layer);

    // TODO FIXME
    selected_layers = layers;

    new_layer = new Layer(RecordMode::DUB, division, selected_layers);
  }

  if (mode != RecordMode::READ) {
    new_layer->write(division, phase, in, attenuation_power);
  }

  advance(sample_time, use_ext_phase, ext_phase);
}


void Section::setRecordMode(RecordMode new_mode) {
  if (mode != RecordMode::READ && new_mode == RecordMode::READ) {
    assert(new_layer != NULL);
    if (new_layer->mode == RecordMode::DEFINE_DIVISION_LENGTH) {
      phase_oscillator.setPitch(1 / (new_layer->samples_per_division * last_sample_time));
      phase_defined = true;
      printf("phase defined with pitch %f, s/div %d, s_time %f\n", phase_oscillator.freq, new_layer->samples_per_division, last_sample_time);
    }

    layers.push_back(new_layer);

    printf("END recording\n");
    printf("-- mode %d, start div: %d, length: %d\n", new_layer->mode, new_layer->start_division, new_layer->n_divisions);

    new_layer = NULL;
  }

  if (mode == RecordMode::READ && new_mode != RecordMode::READ) {
    // TODO FIXME
    selected_layers = layers;
    new_layer = new Layer(new_mode, division, selected_layers);

  }

  mode = new_mode;
}

bool Section::isEmpty() { return (layers.size() == 0); }
