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

// TODO, how to define division length?
// decided not to use clk port - maybe manual entry?
// for now, size of first layer.
unsigned int Frame::Engine::Scene::getDivisionLength() {
  if (layers.size() > 0 && layers[0]->buffer.size() > 0) {
    return layers[0]->buffer.size();
  }

  return 1;
}

bool Frame::Engine::Scene::isEmpty() {
  return (layers.size() == 0);
}

void Frame::Engine::Scene::Layer::attenuate(int phase, float attenuation) {
  if (fully_attenuated) {
    return;
  }

  unsigned int layer_phase = phase % length;
  bool in_recorded_area = (start_division_offset <= layer_phase &&
                           layer_phase <= length + end_division_offset);
  if (in_recorded_area) {
    unsigned int layer_sample_i = layer_phase - start_division_offset;
    float current_attenuation = attenuation_envelope[layer_sample_i];
    attenuation_envelope[layer_sample_i] =
        clamp(current_attenuation + attenuation, 0.0f, 1.0f);
  }
}

void Frame::Engine::Scene::Layer::endRecording(unsigned int phase, unsigned int division_length) {
  recording = false;

  end_division = round(phase / division_length);
  if (end_division == start_division) {
    end_division++;
  }

  if (end_division <= start_division) {
    printf("FRAME:: ERROR, end_division %d > start_division %d", end_division, start_division);
  }

  end_division_offset = phase - end_division * division_length;
  n_divisions = end_division - start_division;
  length = n_divisions * division_length;

  printf("END recording\n");
  printf("-- start_div: %d, start_division_offset: %d\n", start_division, start_division_offset);
  printf("-- end_div: %d, end_division_offset: %d\n", end_division, end_division_offset);
  printf("-- div length: %d\n", division_length);
  printf("-- phase end: %d\n", phase);
  printf("-- buffer length: %ld\n", buffer.size());
}

void Frame::Engine::Scene::Layer::startRecording(vector<Engine::Scene::Layer *> selected_layers, unsigned int phase, unsigned int current_division_length) {
  recording = true;
  division_length = current_division_length;

  for (auto selected_layer : selected_layers) {
    if (selected_layer && !selected_layer->fully_attenuated) {
      target_layers.push_back(selected_layer);
    }
  }

  start_division = floor(phase / division_length);
  start_division_offset = phase - start_division * division_length;

  printf("START recording\n");
  printf("-- phase start: %d\n", phase);
  printf("-- start div: %d, start div offset: %d\n", start_division, start_division_offset);
  printf("-- div length: %d\n", division_length);
}

void Frame::Engine::Scene::addLayer() {
  Engine::Scene::Layer *new_layer = new Engine::Scene::Layer();

  printf("NEW LAYER ~~~~ Mode:: %d\n", mode);

  if (mode != Mode::READ) {
    // TODO
    selected_layers = layers;
    new_layer->startRecording(selected_layers, phase, getDivisionLength());

    if (mode == Mode::ADD && current_layer) {
      // TODO better way? instead of reaching in?
      new_layer->n_divisions = current_layer->n_divisions;
      new_layer->length = current_layer->length;
    } else if (mode == Mode::EXTEND) {
      // so to extend the scene instead of scene phase looping
      n_scene_divisions = -1;
    }
  }

  current_layer = new_layer;
  layers.push_back(new_layer);
}

void Frame::Engine::Scene::Layer::step(unsigned int phase, float in,
                                       float attenuation_power) {
  if (recording) {
    // TODO only if input present

    buffer.push_back(in);
    // TODO attenuation divider, don't need to save every point
    target_attenuation_envelope.push_back(attenuation_power);

    // FIXME
    // for (auto target_layer : target_layers) {
    //   target_layer->attenuate(phase, attenuation_power);
    // }
  }

  // TODO
  // target_layer->attenuation = new_attenuation;
  // if (new_attenuation > 1.0f) {
  //   target_fully_attenuated = false;
  // }
  // target_layer->fully_attenuated = target_fully_attenuated;
}

void Frame::Engine::Scene::step(float in_p, float attenuation_power) {
  phase++;
  for (auto layer : layers) {
    layer->step(phase, in_p, attenuation_power);
  }

  if (mode == Mode::ADD && phase % current_layer->length == 0) {
    current_layer->endRecording(phase, getDivisionLength());
    if (phase >= length) {
      phase = 0;
    }
    addLayer();
  }

  if (mode == Mode::READ && phase >= length) {
    phase = 0;
  }
}

void Frame::Engine::Scene::updateSceneLength() {
    vector<int> lengths;
    for (auto layer : layers) {
      lengths.push_back(layer->end_division - layer->start_division);
    }

    int lc_multiple = accumulate(lengths.begin(), lengths.end(), 1, lcm);
    n_scene_divisions = lc_multiple;
    length = n_scene_divisions * getDivisionLength();
}

void Frame::Engine::Scene::setMode(Mode new_mode) {
  Mode prev_mode = mode;
  mode = new_mode;
  if (prev_mode != Mode::READ && mode == Mode::READ) {
    current_layer->endRecording(phase, getDivisionLength());

    if (prev_mode == Mode::EXTEND) {
      updateSceneLength();
      printf("New Scene N Divisiosn: %d\n", n_scene_divisions);
      printf("New Scene Length: %d\n", length);

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

float Frame::Engine::Scene::Layer::read(unsigned int phase) {
  // don't output what we are writing to avoid FB in case of self-routing
  if (recording || fully_attenuated || length == 0 || buffer.size() == 0) {
    return 0.0f;
  }

  // TODO smooth edges so no clicks
  // FIXME how to handle offset?
  unsigned int layer_phase = phase % length;

  bool in_recorded_area = (start_division_offset <= layer_phase && layer_phase <= length + end_division_offset);
  if (in_recorded_area) {
    int layer_sample_i = layer_phase - start_division_offset;
    // FIXME
    // return buffer[layer_sample_i] * (1 - attenuation_envelope[layer_sample_i]);
    return buffer[layer_sample_i];
  }

  return 0.0f;
}

float Frame::Engine::Scene::Layer::readAttenuation(unsigned int phase) {
  // don't output what we are writing to avoid FB in case of self-routing
  if (target_attenuation_envelope.size() ==  0) {
    return 0.0f;
  }

  if (recording) {
    return target_attenuation_envelope.back();
  }


  // TODO smooth edges so no clicks
  // FIXME how to handle offset?
  unsigned int layer_phase = phase % length;

  bool in_recorded_area = (start_division_offset <= layer_phase &&
                           layer_phase <= length + end_division_offset);
  if (in_recorded_area) {
    int layer_sample_i = layer_phase - start_division_offset;
    return target_attenuation_envelope[layer_sample_i];
  }

  return 0.0f;
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

    float layer_out = layers[i]->read(phase);

    // FIXME have rendered envelope to save cpu
    float layer_attenuation = 0.0f;
    for (unsigned int j = i + 1; j < layers.size(); j++) {
      for (auto target_layer : layers[j]->target_layers) {
        if (target_layer == layers[i]) {
          layer_attenuation += layers[j]->readAttenuation(phase);
        }
      }
    }

    layer_attenuation = clamp(layer_attenuation, 0.0f, 1.0f);
    out += layer_out * (1 - layer_attenuation);
  }

  return out;
}
