#include "Frame.hpp"

float Frame::Engine::Scene::Layer::read(unsigned int phase,
                                        unsigned int division_length) {
  float out = 0.0f;
  unsigned int layer_length = (end_division - start_division) * division_length;

  if (buffer.size() > 0) {
    // TODO smooth edges so no clicks
    // FIXME how to handle offset?
    unsigned int layer_phase = phase % layer_length;

    if (start_division_offset <= layer_phase &&
        layer_phase <= layer_length + end_division_offset) {
      unsigned int layer_sample_i = layer_phase - start_division_offset;
      out = buffer[layer_sample_i];
    }
  }

  return out;
}

float Frame::Engine::Scene::Layer::readAttenuation(int phase, int division_length) {
  float out = 0.0f;
  unsigned int layer_length = (end_division - start_division) * division_length;

  if (attenuation_envelope.size() > 0) {
    // TODO smooth edges so no clicks
    // FIXME how to handle offset?
    unsigned int layer_phase = phase % layer_length;

    if (start_division_offset <= layer_phase &&
        layer_phase <= layer_length + end_division_offset) {
      unsigned int layer_sample_i = layer_phase - start_division_offset;
      out = attenuation_envelope[layer_sample_i];
    }
  }

  return out;
}

float Frame::Engine::Scene::read(float attenuation_power) {
  float out = 0.0f;
  for (unsigned int i=0; i < layers.size(); i++) {
    // don't output what we are writing to avoid FB in case of self-routing
    bool layer_is_recording = (mode != Mode::READ && layers[i] == current_layer);
    if (layer_is_recording || layers[i]->fully_attenuated) {
      continue;
    }

    float layer_out = layers[i]->read(phase, getDivisionLength());

    // FIXME
    float layer_attenuation = 0.0f;
    for (unsigned int j=i+1; j < layers.size(); j++) {
      for (auto target_layer : layers[j]->target_layers) {
        if (target_layer == layers[i]) {
          // FIXME hack to offset sample to attenuate live
          if (mode != Mode::READ && layers[j] == current_layer && phase != 0) {
            layer_attenuation += attenuation_power;
          } else {
            layer_attenuation += layers[j]->readAttenuation(phase);
          }

          break;
        }
      }
    }

    // TODO attenuation envelopes
    layer_attenuation = clamp(layer_attenuation, 0.0f, 1.0f);
    layer_out *= (1 - layer_attenuation);
    if (layer_out > 0.00f) {
      layers[i]->attenuation_flag = false;
    }

    if (phase == layers[i]->length - 1) {
      if (layers[i]->attenuation_flag) {
        layers[i]->fully_attenuated = true;
      } else {
        layers[i]->attenuation_flag = true;
      }
    }

    out += layer_out;
  }

  return out;
}

