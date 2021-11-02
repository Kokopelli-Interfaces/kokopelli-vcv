#pragma once

#include "rack.hpp"
#include <thread>

namespace kokopellivcv {
namespace dsp {

struct LightBlinker {
  std::chrono::time_point<std::chrono::system_clock> _time;
  bool _op = false;
  std::vector<rack::engine::Light> *_lights;
  int _light_i;
  std::vector<float> _saved_light_brightness = {0.f, 0.f, 0.f};

  LightBlinker(std::vector<rack::engine::Light> *lights) {
    _lights = lights;
  }

  void finishBlinkLight() {
    _op = false;
    for (int i = 0; i < 3; i++) {
      _lights->operator[](_light_i + i).value = _saved_light_brightness[i];
    }
  }

  void blinkLight(int light_i) {
    if (_op) {
      finishBlinkLight();
    }

    _light_i = light_i;
    _time = std::chrono::system_clock::now();
    _op = true;

    for (int i = 0; i < 3; i++) {
      _saved_light_brightness[i] = _lights->operator[](light_i + i).value;
    }

    _lights->operator[](_light_i + 0).value = _lights->operator[](_light_i + 0).value * 2.f;
    _lights->operator[](_light_i + 1).value = _lights->operator[](_light_i + 1).value * 2.f;
    _lights->operator[](_light_i + 2).value = _lights->operator[](_light_i + 2).value * 2.f;
  }

  void step() {
    if (_op) {
      auto now = std::chrono::system_clock::now();
      if (now - _time > std::chrono::milliseconds{50}) {
        finishBlinkLight();
      }
    }
  }
};

} // namespace dsp
} // namespace kokopellivcv
