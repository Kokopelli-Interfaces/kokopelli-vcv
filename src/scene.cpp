#include "Frame.hpp"

bool Frame::Engine::Scene::isEmpty() {
  return (layers.size() == 0);
}

void Frame::Engine::Scene::addLayer() {
  Engine::Scene::Layer *new_layer = new Engine::Scene::Layer();

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

void Frame::Engine::Scene::Layer::addDivision() {
  vector<float> division_buffer(samples_per_division, 0.0f);
  division_buffers.push_back(division_buffer);

  vector<float> division_attenuation_send(samples_per_division, 0.0f);
  division_attenuation_sends.push_back(division_attenuation_send);

  divisions++;

  if (divisions > length) {
    length = divisions;
  }
}

void Frame::Engine::Scene::Layer::write(unsigned int current_division, float phase, float sample, float send_attenuation) {
  if (divisions == 0) {
    addDivision();
  }

  if (define_division_length) {
    division_buffers[0].push_back(sample);
    division_attenuation_sends[0].push_back(send_attenuation);
  } else {
    unsigned int division = current_division - start_division;
    if (divisions <= division) {
      addDivision();
    }

    // TODO what if phase == 1?
    int division_sample_i = floor(samples_per_division * phase);

    division_buffers[division][division_sample_i] = sample;
    // TODO divider
    division_attenuation_sends[division][division_sample_i] = send_attenuation;
  }
}


float Frame::Engine::Scene::Layer::readGeneric(unsigned int current_division, float phase, bool read_attenuation) {
  if (current_division < start_division) {
    return 0.0f;
  }

  int layer_division = (current_division - start_division) % length;
  if (layer_division < divisions) {
    int division_sample_i = floor(samples_per_division * phase);
    if (read_attenuation) {
      return division_attenuation_sends[layer_division][division_sample_i];
    } else {
      return division_buffers[layer_division][division_sample_i];
    }
  }

  return 0.0f;
}

float Frame::Engine::Scene::Layer::readSample(unsigned int current_division, float phase) {
  if (fully_attenuated) {
      return 0.0f;
  }

  return readGeneric(current_division, phase, false);
}

float Frame::Engine::Scene::Layer::readAttenuation(unsigned int current_division, float phase) {
  return readGeneric(current_division, phase, true);
}

float Frame::Engine::Scene::read() {
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

    layer_attenuation = clamp(layer_attenuation, 0.0f, 1.0f);
    out += layer_out * (1 - layer_attenuation);
  }

  return out;
}

void Frame::Engine::Scene::setExtPhase(float ext_phase) {
  float delta = ext_phase - last_ext_phase;
  float abs_delta = fabsf(delta);
  ext_phase_flipped = (abs_delta > 9.5f && abs_delta <= 10.0f);
}

bool Frame::Engine::Scene::stepPhase(float sample_time) {
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

float Frame::Engine::Scene::getPhase() {
  if (ext_phase) {
    return last_ext_phase;
  }

  if (isEmpty() || (current_layer->define_division_length && mode != Mode::READ)) {
    return 0.0f;
  }

  return phase_oscillator.getPhase();
}

void Frame::Engine::Scene::step(float in, float attenuation_power, float sample_time) {
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

void Frame::Engine::Scene::setMode(Mode new_mode, float sample_time) {
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
  }

  if (prev_mode == Mode::READ && mode != Mode::READ) {
    addLayer();
  }
}

Frame::Mode Frame::Engine::Scene::getMode() {
  return mode;
}
