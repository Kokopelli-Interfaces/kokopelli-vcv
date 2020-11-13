#include "Scene.hpp"

using namespace myrisa;

bool Scene::isEmpty() {
  return (layers.size() == 0);
}

void Scene::addLayer() {
  ASSERT(mode, !=, Mode::READ);

  antipop_filter.trigger();

  Layer *new_layer = new Layer();

  // TODO FIXME have this selectable and depend on mode
  selected_layers = layers;

  for (auto selected_layer : selected_layers) {
    if (selected_layer && !selected_layer->fully_attenuated) {
      new_layer->target_layers.push_back(selected_layer);
    }
  }

  new_layer->start_division = division;

  if (mode == Mode::ADD) {
    auto most_recent_target_layer = new_layer->target_layers.back();
    if (most_recent_target_layer) {
      new_layer->length = most_recent_target_layer->length;
    } else {
      new_layer->length = 1;
    }
  }

  new_layer->samples_per_division = samples_per_division;
  if (isEmpty()) {
    new_layer->define_division_length = true;
  }

  current_layer = new_layer;
  layers.push_back(new_layer);

  printf("START recording\n");
}

float Scene::read(float sample_time) {
  float out = 0.0f;
  for (unsigned int i = 0; i < layers.size(); i++) {
    // don't output what we are writing to avoid FB in case of self-routing
    if (mode != Mode::READ && layers[i] == current_layer) {
      continue;
    }

    float layer_out = layers[i]->readSample(division, phase, sample_time);

    // FIXME clk divide to save cpu
    float layer_attenuation = 0.0f;
    for (unsigned int j = i + 1; j < layers.size(); j++) {
      for (auto target_layer : layers[j]->target_layers) {
        if (target_layer == layers[i]) {
          layer_attenuation += layers[j]->readAttenuation(division, phase, sample_time);
        }
      }
    }

    layer_attenuation = rack::clamp(layer_attenuation, 0.0f, 1.0f);
    out += layer_out * (1 - layer_attenuation);
  }

  return out;
}

void Scene::step(float in, float attenuation_power, float sample_time, bool use_ext_phase, float ext_phase) {
  last_phase = phase;
  if (use_ext_phase) {
    ASSERT(0, <=, ext_phase);
    ASSERT(ext_phase, <=, 1.0f);
    phase = ext_phase;
    phase_defined = true;
  } else if (!phase_defined) {
    phase = 0.0f;
  } else {
    phase_oscillator.step(sample_time);
    phase = phase_oscillator.getPhase();
  }

  float delta = phase - last_phase;
  bool phase_flip = (fabsf(delta) > 0.95f && fabsf(delta) <= 1.0f);
  if (phase_flip) {
    antipop_filter.trigger();

    if (0 < delta && 0 < division) {
      division--;
    } else if (delta < 0) {
      division++;
    }

    if (mode == Mode::ADD) {
      unsigned int layer_end_division = current_layer->start_division + current_layer->length;
      if (layer_end_division == division) {
        printf("END recording via overdub\n");
        printf("-- start div: %d, length: %d\n", current_layer->start_division, current_layer->length);
        endRecording(sample_time);
        addLayer();
      }
    }
  }

  if (mode != Mode::READ) {
    float layer_in = antipop_filter.process(in, sample_time);
    current_layer->write(division, phase, layer_in, attenuation_power);
  }
}

void Scene::endRecording(float sample_time) {
  // FIXME  just using last layer as example
  if (!isEmpty() && current_layer->define_division_length) {
    samples_per_division = current_layer->samples_per_division;
    phase_oscillator.setPitch(1 / (samples_per_division * sample_time));
    phase_defined = true;
  }
}

void Scene::setMode(Mode new_mode, float sample_time) {
  Mode prev_mode = mode;
  mode = new_mode;
  if (prev_mode != Mode::READ && mode == Mode::READ) {
    printf("END recording\n");
    printf("-- start div: %d, length: %d\n", current_layer->start_division, current_layer->length);

    if (prev_mode == Mode::EXTEND) {
      if (phase <= 0.50f && current_layer->length > 1) {
        current_layer->length--;
      }
    }

    endRecording(sample_time);
  }

  if (prev_mode == Mode::READ && mode != Mode::READ) {
    addLayer();
  }
}

Scene::Mode Scene::getMode() {
  return mode;
}
