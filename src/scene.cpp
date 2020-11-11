#include "Frame.hpp"

int gcd(int a, int b) {
  for (;;) {
    if (a == 0)
      return b;
    b %= a;
    if (b == 0)
      return a;
    a %= b;
  }
}

int lcm(int a, int b) {
  int temp = gcd(a, b);
  return temp ? (a / temp * b) : 0;
}

bool Frame::Engine::Scene::isEmpty() {
  return (layers.size() == 0);
}

void Frame::Engine::Scene::Layer::endRecording() {
  recording = false;
  printf("END recording\n");
  printf("-- start div: end division: %d\n", start_division, start_division + divisions);
}

void Frame::Engine::Scene::Layer::startRecording(vector<Engine::Scene::Layer *> selected_layers, unsigned int current_division) {
  recording = true;

  for (auto selected_layer : selected_layers) {
    if (selected_layer && !selected_layer->fully_attenuated) {
      target_layers.push_back(selected_layer);
    }
  }

  start_division = current_division;

  printf("START recording\n");
  printf("-- start div: %d", start_division);
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
  new_layer->startRecording(selected_layers, division);

  if (mode == Mode::ADD && current_layer) {
    // TODO better way? instead of reaching in?
    new_layer->divisions = current_layer->divisions;
  } else if (mode == Mode::EXTEND) {
    // TODO ???
    // new_layer->divisions = -1;
  }

  current_layer = new_layer;
  layers.push_back(new_layer);
}

void Frame::Engine::Scene::Layer::addDivision(int samples_per_division) {
  vector<float> division_buffer(samples_per_division, 0.0f);
  division_buffers.push_back(division_buffer);

  vector<float> division_attenuation_send(samples_per_division, 0.0f);
  division_attenuation_sends.push_back(division_attenuation_send);

  divisions++;
}

void Frame::Engine::Scene::Layer::write(unsigned int division, float phase, float sample, float send_attenuation, int samples_per_division) {
  if (undefined_phase_length) {
    if (divisions == 0) {
      addDivision(0);
    }

    division_buffers[0].push_back(in);
    division_attenuation_sends[0].push_back(attenuation);
  } else {
    int layer_division = division - start_division;
    if (divisions == layer_division) {
      addDivision(samples_per_division);
    }

    division_buffers[layer_division][division_sample_i] = in;
    // TODO divider
    division_attenuation_sends[layer_division][division_sample_i] = send_attenuation;
  }
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

  return phase_oscillator.step(sample_time);
}

float Frame::Engine::Scene::getPhase() {
  if (ext_phase) {
    return last_ext_phase;
  }

  return phase_oscillator->getPhase();
}

void Frame::Engine::Scene::step(float in, float attenuation_power, float sample_time) {
  bool phase_flip = this->stepPhase();
  float phase = this->getPhase();

  if (phase_flip) {
    division++;

    int layer_end_division = current_layer->start_division + current_layer->divisions;
    if (mode == Mode::ADD && layer_end_division == division) {
      current_layer->endRecording();
      this->addLayer();
    }
    // TODO if reached lcm, reset
  }

  if (current_layer->recording) {
    current_layer->write(division, phase, in, attenuation_power, samples_per_division);
  }
}

// void Frame::Engine::Scene::updateSceneLength() {
//     vector<int> lengths;
//     for (auto layer : layers) {
//       lengths.push_back(layer->end_division - layer->start_division);
//     }

//     int lc_multiple = accumulate(lengths.begin(), lengths.end(), 1, lcm);
//     n_scene_divisions = lc_multiple;
//     length = n_scene_divisions * getDivisionLength();
// }

void Frame::Engine::Scene::setMode(Mode new_mode) {
  Mode prev_mode = mode;
  mode = new_mode;
  if (prev_mode != Mode::READ && mode == Mode::READ) {
    current_layer->endRecording();

    // if (prev_mode == Mode::EXTEND) {
      // updateSceneLength();
      // printf("New Scene N Divisiosn: %d\n", n_scene_divisions);
      // printf("New Scene Length: %d\n", length);

      // the scene phase may reset, so we do not want a mini skip back to occur
      // in the case the new layers length is rounded down
      // if (current_layer->end_division_offset > 0 && current_layer->end_division * current_layer->division_length >= n_scene_divisions) {
      //   phase = 0 + current_layer->end_division_offset;
      // }
    }
  }

  if (prev_mode == Mode::READ && mode != Mode::READ) {
    addLayer();
  }
}

Frame::Mode Frame::Engine::Scene::getMode() {
  return mode;
}

float Frame::Engine::Scene::Layer::readGeneric(unsigned int current_division, float phase, bool read_attenuation) {
  int layer_division = current_division % divisions;

  if (in_recorded_area) {
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
  // don't output what we are writing to avoid FB in case of self-routing
  if (recording || fully_attenuated) {
    return 0.0f;
  }

  return readGeneric(current_division, phase, false);
}

float Frame::Engine::Scene::Layer::readAttenuation(unsigned int current_division, float phase) {
  if (division_attenuation_sends.size() == 0) {
    return 0.0f;
  }

  return readGeneric(current_division, phase, true);
}

float Frame::Engine::Scene::read() {
  float out = 0.0f;
  for (unsigned int i = 0; i < layers.size(); i++) {
    // don't output what we are writing to avoid FB in case of self-routing
    bool layer_is_recording =
    (mode != Mode::READ && layers[i] == current_layer);
    if (layer_is_recording || layers[i]->fully_attenuated) {
      continue;
    }

    float layer_out = layers[i]->readSample(division, phase);

    // FIXME have rendered envelope to save cpu
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
