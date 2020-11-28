#include "Scene.hpp"

using namespace myrisa::dsp::frame;

Scene::~Scene() {
  for (auto layer : _layers) {
    delete layer;
  }

  if (_active_layer) {
    delete _active_layer;
  }
}

// FIXME performance
float Scene::getLayerAttenuation(int layer_i, float current_attenuation) {
  float layer_attenuation = 0.0f;
  if (_active_layer) {
    for (auto target_layer : _active_layer->target_layers) {
      if (target_layer == _layers[layer_i]) {
        layer_attenuation += current_attenuation;
        break;
      }
    }
  }

  if (1.0f <= layer_attenuation) {
    return 1.0f;
  }

  for (unsigned int j = layer_i + 1; j < _layers.size(); j++) {
    for (auto target_layer : _layers[j]->target_layers) {
      if (target_layer == _layers[layer_i]) {
        layer_attenuation += _layers[j]->readSendAttenuation(_scene_division, _phase);
        if (1.0f <= layer_attenuation) {
          return 1.0f;
        }
      }
    }
  }

  return layer_attenuation;
}

float Scene::read(float current_attenuation) {
  float out = 0.0f;
  for (unsigned int i = 0; i < _layers.size(); i++) {
    float layer_attenuation = getLayerAttenuation(i, current_attenuation);
    if (layer_attenuation < 1.0f) {
      float layer_out = _layers[i]->readSampleWithAttenuation(_scene_division, _phase, layer_attenuation);
      out += layer_out;
    }
  }

  return out;
}

bool Scene::isEmpty() { return (_layers.size() == 0); }