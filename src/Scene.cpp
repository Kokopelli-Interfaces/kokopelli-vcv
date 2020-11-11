#include "Scene.hpp"

using namespace myrisa;

bool Scene::isEmpty() {
  return (layers.size() == 0);
}

void Scene::addLayer() {
  Layer *new_layer = new Layer();

  printf("NEW LAYER ~~~~ Mode:: %d\n", mode);
  if (mode == Mode::READ) {
    printf("FRAME ERROR: Add layer was called but scene is in READ mode.");
    return;
  }

  // TODO
  selected_layers = layers;

  for (auto selected_layer : selected_layers) {
    if (selected_layer && !selected_layer->fully_attenuated) {
      new_layer->target_layers.push_back(selected_layer);
    }
  }

  new_layer->start_division = division;
  auto most_recent_target_layer = new_layer->target_layers.back();
  if (mode == Mode::ADD && most_recent_target_layer) {
    new_layer->length = most_recent_target_layer->length;
  }

  new_layer->samples_per_division = samples_per_division;
  if (isEmpty() && !ext_phase) {
    new_layer->define_division_length = true;
  }

  current_layer = new_layer;
  layers.push_back(new_layer);

  printf("START recording\n");
}

float Scene::read() {
  float out = 0.0f;
  float phase = getPhase();
  for (unsigned int i = 0; i < layers.size(); i++) {
    // don't output what we are writing to avoid FB in case of self-routing
    if (mode != Mode::READ && layers[i] == current_layer) {
      continue;
    }

    float layer_out = layers[i]->readSample(division, phase);

    // FIXME clk divide to save cpu
    float layer_attenuation = 0.0f;
    for (unsigned int j = i + 1; j < layers.size(); j++) {
      for (auto target_layer : layers[j]->target_layers) {
        if (target_layer == layers[i]) {
          layer_attenuation += layers[j]->readAttenuation(division, phase);
        }
      }
    }

    layer_attenuation = rack::clamp(layer_attenuation, 0.0f, 1.0f);
    out += layer_out * (1 - layer_attenuation);
  }

  return out;
}

void Scene::setExtPhase(float ext_phase) {
  float delta = ext_phase - last_ext_phase;
  float abs_delta = fabsf(delta);
  ext_phase_flipped = (abs_delta > 9.5f && abs_delta <= 10.0f);
}

bool Scene::stepPhase(float sample_time) {
  if (ext_phase) {
    if (ext_phase_flipped) {
      ext_phase_flipped = false;
      return true;
    }

    return false;
  }

  if (isEmpty() || (current_layer->define_division_length && mode != Mode::READ)) {
    return false;
  }

  return phase_oscillator.step(sample_time);
}

float Scene::getPhase() {
  if (ext_phase) {
    return last_ext_phase;
  }

  if (isEmpty() || (current_layer->define_division_length && mode != Mode::READ)) {
    return 0.0f;
  }

  return phase_oscillator.getPhase();
}

void Scene::step(float in, float attenuation_power, float sample_time) {
  bool phase_flip = stepPhase(sample_time);
  float phase = getPhase();

  if (phase_flip) {
    division++;

    unsigned int layer_end_division = current_layer->start_division + current_layer->divisions;
    if (mode == Mode::ADD && layer_end_division == division) {
      printf("END recording\n");
      printf("-- start div: %d, end division: %d\n", current_layer->start_division, layer_end_division);
      printf("-- divisions: %d, length: %d\n",
             current_layer->divisions, current_layer->length);
      addLayer();
    }
    // TODO if reached lcm, reset
  }

  if (mode != Mode::READ) {
    current_layer->write(division, phase, in, attenuation_power);
  }
}

void Scene::setMode(Mode new_mode, float sample_time) {
  Mode prev_mode = mode;
  mode = new_mode;
  if (prev_mode != Mode::READ && mode == Mode::READ) {
    printf("END recording\n");
    printf("-- start div: %d, divisions: %d\n", current_layer->start_division, current_layer->divisions);
    printf("-- divisions: %d, length: %d\n", current_layer->divisions,
           current_layer->length);

    if (current_layer->define_division_length) {
      samples_per_division = current_layer->division_buffers[0].size();
      current_layer->samples_per_division = samples_per_division;
      phase_oscillator.setPitch(1 / (samples_per_division * sample_time));
      // phase_oscillator.reset(0.0f);
    }

    if (prev_mode == Mode::EXTEND) {
      if (getPhase() <= 0.50f && current_layer->length > 1) {
        current_layer->length--;
      }
    }
  }

  if (prev_mode == Mode::READ && mode != Mode::READ) {
    addLayer();
  }
}

Scene::Mode Scene::getMode() {
  return mode;
}
