#include "Scene.hpp"

using namespace myrisa;

bool Scene::isEmpty() {
  return (layers.size() == 0);
}

void Scene::startNewLayer(Mode layer_mode) {
  ASSERT(layer_mode, !=, Mode::READ);

  // TODO FIXME have this selectable and depend on mode
  selected_layers = layers;

  float length;
  float start_division;
  if (layer_mode == Mode::DUB) {
    if (fmod(position, 1.0f) < 0.95f) {
      start_division = floor(position);
    } else {
      start_division = ceil(position);
    }
    auto most_recent_target_layer = selected_layers.back();
    if (most_recent_target_layer) {
      length = most_recent_target_layer->length;
    } else {
      length = 1;
    }
  } else {
    start_division = round(position);
    length = 1;
  }
  new_layer = new Layer(start_division, length);

  for (auto selected_layer : selected_layers) {
    if (selected_layer && !selected_layer->fully_attenuated) {
      new_layer->target_layers.push_back(selected_layer);
    }
  }

  // layers.push_back(new_layer);
  printf("START recording, start div: %f, len %f\n", start_division, length);
}

// FIXME performance
float Scene::getLayerAttenuation(int layer_i) {
  float layer_attenuation = 0.0f;
  // for (unsigned int j = layer_i + 1; j < layers.size(); j++) {
  //   for (auto target_layer : layers[j]->target_layers) {
  //     if (target_layer == layers[layer_i]) {
  //       layer_attenuation += layers[j]->readSendAttenuation(position);
  //     }
  //   }
  // }

  // if (new_layer) {
  //   for (auto target_layer : new_layer->target_layers) {
  //     if (target_layer == layers[layer_i]) {
  //       layer_attenuation += target_layer->readSendAttenuation(position);
  //     }
  //   }
  // }

  return layer_attenuation;
}

float Scene::read() {
  float out = 0.0f;
  for (unsigned int i = 0; i < layers.size(); i++) {
    float layer_attenuation = getLayerAttenuation(i);
    float layer_out = layers[i]->readSampleWithAttenuation(position, layer_attenuation);
    out += layer_out;
  }

  return out;
}

void Scene::advancePosition(float sample_time, bool use_ext_phase, float ext_phase) {
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

  float division = position == 1.0f ? floor(position - 0.01f) : floor(position);
  if (phase_flip) {
    if (0 < phase_change && 0 < division) {
      division--;
    } else if (phase_change < 0) {
      division++;
    }
  }

  position = division + phase;
}

void Scene::step(float in, float attenuation_power, float sample_time, bool use_ext_phase, float ext_phase) {
  float division = position == 1.0f ? floor(position - 0.01f) : floor(position);

  if (mode == Mode::EXTEND) {
    assert(new_layer != NULL);
    if (0.50f < phase && new_layer->start_division + new_layer->length <= division) {
      new_layer->length++;
    }
  } else if (mode == Mode::DUB) {
    assert(new_layer != NULL);
    if (new_layer->start_division + new_layer->length == division) {
      printf("END recording via overdub\n");
      printf("-- start div: %f, length: %f\n", new_layer->start_division, new_layer->length);
      finishNewLayer();
      startNewLayer(Mode::DUB);
    }
  }

  if (mode != Mode::READ) {
    assert(new_layer != NULL);

    if (mode == Mode::DEFINE_DIVISION) {
      new_layer->writeByCreatingDivision(in, attenuation_power);
    } else {
      new_layer->write(position, in, attenuation_power);
    }

    // FIXME do not need to set every loop..
    new_layer->sample_time = sample_time;
  }

  advancePosition(sample_time, use_ext_phase, ext_phase);
}

void Scene::finishNewLayer() {
  assert(new_layer != NULL);
  // FIXME  just using last layer as example
  if (isEmpty() || mode == Mode::DEFINE_DIVISION) {
    int samples_per_division = new_layer->num_samples;
    ASSERT(1, <, samples_per_division);
    phase_oscillator.setPitch(1 / (samples_per_division * new_layer->sample_time));
    phase_defined = true;
    printf("phase defined\n");
  }

  printf("END recording\n");
  printf("-- start div: %f, length: %f\n", new_layer->start_division,
         new_layer->length);

  layers.push_back(new_layer);
  new_layer = NULL;
}

void Scene::setMode(Mode new_mode) {
  if (mode != Mode::READ && new_mode == Mode::READ) {
    finishNewLayer();
  }

  if (mode == Mode::READ && new_mode != Mode::READ) {
    startNewLayer(new_mode);
  }

  mode = new_mode;
}