#include "Aion.hpp"
#include "AionWidget.hpp"

Aion::Aion() {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  configParam(PREV_MOVEMENT_PARAM, 0.f, 1.f, 0.f, "Previous Movement");

  _light_divider.setDivision(512);
  _button_divider.setDivision(4);
  _light_blinker = new kokopellivcv::dsp::LightBlinker(&lights);

  _prev_movement_button.param = &params[PREV_MOVEMENT_PARAM];
}

void Aion::sampleRateChange() {
  _sample_time = APP->engine->getSampleTime();
}

void Aion::processButtons(const ProcessArgs &args) {
  float sample_time = _sample_time * _button_divider.division;

  kokopellivcv::dsp::LongPressButton::Event _prev_movement_event = _prev_movement_button.process(sample_time);

  for (int c = 0; c < _connected_circle->channels(); c++) {
    switch (_prev_movement_event) {
    case kokopellivcv::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopellivcv::dsp::LongPressButton::SHORT_PRESS:
      _light_blinker->blinkLight(PREV_MOVEMENT_LIGHT, 3.f);
      _connected_circle->_engines[c]->voiceBackward();
      break;
    case kokopellivcv::dsp::LongPressButton::LONG_PRESS:
      _light_blinker->blinkLight(PREV_MOVEMENT_LIGHT, 0.f); // TODO blink extra extra
      _connected_circle->_engines[c]->toggleMovementProgression();
      break;
    }
  }
}

bool Aion::isNextToHearth() {
  // FIXME how to check if its circle?
  if (this->rightExpander.module && this->rightExpander.module->model == modelHearth) {
    return true;
  }
  return false;
}

void Aion::connect() {
  // FIXME find right circle
  printf("CSIZE %ld\n", CIRCLES.size());
  if (!CIRCLES.empty()) {
    _connected_circle = CIRCLES[0];
    printf("Aion connected!\n");
  }
}

void Aion::processAlways(const ProcessArgs &args) {
  if (_button_divider.process()) {
    if (isNextToHearth() && !_connected_circle) {
      connect();
    } else if (!isNextToHearth() && _connected_circle) {
      _connected_circle = nullptr;
    }

    if (_connected_circle) {
      processButtons(args);
    }
  }
}

void Aion::updateLight(int light, NVGcolor color, float strength) {
  if (_light_blinker->_op && _light_blinker->_light_i == light) {
    return;
  }

  lights[light + 0].value = color.r * strength;
  lights[light + 1].value = color.g * strength;
  lights[light + 2].value = color.b * strength;
}

void Aion::postProcessAlways(const ProcessArgs &args) {
  if (_light_divider.process()) {
    updateLights(args);
  }
}

void Aion::updateLights(const ProcessArgs &args) {
  _light_blinker->step();

  float light_strength = 0.0f;
  if (_connected_circle) {
    light_strength = .3f;
  }
  updateLight(PREV_MOVEMENT_LIGHT, colors::OBSERVED_SUN_LIGHT, light_strength);
}

Model* modelAion = rack::createModel<Aion, AionWidget>("KokopelliInterfaces-Aion");
