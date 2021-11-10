#include "Circle.hpp"
#include "CircleWidget.hpp"

Circle::Circle() {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  configParam(TUNE_PARAM, 0.f, 1.f, 0.f, "Tune to Group Frequency");
  configParam(ASCEND_PARAM, 0.f, 1.f, 0.f, "Ascend");
  configParam(PROGRESS_PARAM, 0.f, 1.f, 0.f, "Progress");
  configParam(LOVE_PARAM, 0.f, 1.f, 0.f, "Love Direction");

  _light_divider.setDivision(512);
  _button_divider.setDivision(4);
  _light_blinker = new kokopellivcv::dsp::LightBlinker(&lights);

  _tune_button.param = &params[TUNE_PARAM];
  _progress_button.param = &params[PROGRESS_PARAM];
  _ascend_button.param = &params[ASCEND_PARAM];
}

Circle::~Circle() {
  delete _light_blinker;
}

void Circle::sampleRateChange() {
  _sampleTime = APP->engine->getSampleTime();
}

void Circle::processButtons(const ProcessArgs &args) {
  float sampleTime = _sampleTime * _button_divider.division;

  kokopellivcv::dsp::LongPressButton::Event _tune_event = _tune_button.process(sampleTime);
  kokopellivcv::dsp::LongPressButton::Event _progress_event = _progress_button.process(sampleTime);
  kokopellivcv::dsp::LongPressButton::Event _ascend_event = _ascend_button.process(sampleTime);

  for (int c = 0; c < channels(); c++) {
    kokopellivcv::dsp::circle::Engine *e = _engines[c];

    switch (_tune_event) {
    case kokopellivcv::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopellivcv::dsp::LongPressButton::SHORT_PRESS:
      e->toggleTuneToFrequencyOfEstablished();
      updateLights(args);
      _light_blinker->blinkLight(TUNE_LIGHT);
      break;
    case kokopellivcv::dsp::LongPressButton::LONG_PRESS:
      // e->toggleCycleMode();
      break;
    }

    switch (_ascend_event) {
    case kokopellivcv::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopellivcv::dsp::LongPressButton::SHORT_PRESS:
      e->ascend();
      _light_blinker->blinkLight(ASCEND_LIGHT);
      break;
    case kokopellivcv::dsp::LongPressButton::LONG_PRESS:
      e->descend();
      _light_blinker->blinkLight(ASCEND_LIGHT); // TODO extra
      break;
    }

    switch (_progress_event) {
    case kokopellivcv::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopellivcv::dsp::LongPressButton::SHORT_PRESS:
      e->progress();
      _light_blinker->blinkLight(PROGRESS_LIGHT);
      break;
    case kokopellivcv::dsp::LongPressButton::LONG_PRESS:
      e->regress();
      _light_blinker->blinkLight(PROGRESS_LIGHT); // TODO blink extra extra
      break;
    }
  }
}

void Circle::processAlways(const ProcessArgs &args) {
  outputs[PHASE_OUTPUT].setChannels(this->channels());
  outputs[SUN].setChannels(this->channels());

  if (_button_divider.process()) {
    processButtons(args);
  }
}

void Circle::modulate() {
  return;
}

void Circle::modulateChannel(int channel_i) {
  kokopellivcv::dsp::circle::Engine *e = _engines[channel_i];
  float love = params[LOVE_PARAM].getValue();
  if (inputs[LOVE_INPUT].isConnected()) {
    love *= rack::clamp(inputs[LOVE_INPUT].getPolyVoltage(channel_i) / 10.f, 0.f, 1.0f);
  }

  // taking to the strength of 2 gives a more intuitive curve
  love = pow(love, 2);
  e->inputs.love = love;

  e->song.use_ext_phase = inputs[PHASE_INPUT].isConnected();
  // FIXME ugh
  e->options = _options;
  e->song.love_resolution = _options.love_resolution;
  // e->_signal_type = _from_signal->signal_type;
}

int Circle::channels() {
  int input_channels = inputs[WOMB_INPUT].getChannels();
  if (_channels < input_channels) {
    return input_channels;
  }

  return _channels;
}

void Circle::processChannel(const ProcessArgs& args, int channel_i) {
  kokopellivcv::dsp::circle::Engine *e = _engines[channel_i];

  if (inputs[PHASE_INPUT].isConnected()) {
    float phase_in = inputs[PHASE_INPUT].getPolyVoltage(channel_i);
    if (_options.bipolar_phase_input) {
      phase_in += 5.0f;
    }

    e->song.ext_phase = rack::clamp(phase_in / 10, 0.f, 1.0f);
  }

  if (outputs[PHASE_OUTPUT].isConnected()) {
    outputs[PHASE_OUTPUT].setVoltage(e->song.position.phase * 10, channel_i);
  }

  if (inputs[WOMB_INPUT].isConnected()) {
    e->inputs.in = inputs[WOMB_INPUT].getPolyVoltage(channel_i);
  } else {
    e->inputs.in = 0.f;
  }

  e->step();

  if (outputs[SUN].isConnected()) {
    outputs[SUN].setVoltage(e->readAll(), channel_i);
  }

  if (outputs[ESTABLISHED_OUTPUT].isConnected()) {
    outputs[ESTABLISHED_OUTPUT].setVoltage(e->readEstablished(), channel_i);
  }
}

// void Circle::updateLight(int light, float r, float g, float b) {
//   if (_light_blinker->_op && _light_blinker->_light_i == light) {
//     return;
//   }

//   lights[light + 0].value = r;
//   lights[light + 1].value = g;
//   lights[light + 2].value = b;
// }

void Circle::updateLight(int light, NVGcolor color, float strength) {
  if (_light_blinker->_op && _light_blinker->_light_i == light) {
    return;
  }

  lights[light + 0].value = color.r * strength;
  lights[light + 1].value = color.g * strength;
  lights[light + 2].value = color.b * strength;
}


void Circle::updateLights(const ProcessArgs &args) {
  _light_blinker->step();

  kokopellivcv::dsp::circle::Engine *default_e = _engines[0];

  float new_sum = 0.f;
  float established_sum = 0.f;
  for (int c = 0; c < channels(); c++) {
    new_sum += inputs[WOMB_INPUT].getPolyVoltage(c);
    established_sum += outputs[ESTABLISHED_OUTPUT].getPolyVoltage(c);
  }
  new_sum = rack::clamp(new_sum, 0.f, 1.f);
  established_sum = rack::clamp(established_sum, 0.f, 1.f);
  established_sum = established_sum * (1.f - default_e->inputs.love);

  LoveDirection love_direction = default_e->_love_direction;
  if (default_e->_tune_to_frequency_of_established) {
    updateLight(TUNE_LIGHT, colors::ESTABLISHED_LIGHT, default_e->getPhaseOfEstablished());
  } else {
    updateLight(TUNE_LIGHT, colors::WOMB_LIGHT, 0.0f);
  }

  if (love_direction == LoveDirection::ESTABLISHED) {
    updateLight(ASCEND_LIGHT, colors::ESTABLISHED_LIGHT, 0.6f);
    updateLight(PROGRESS_LIGHT, colors::EMERGENCE_LIGHT, 0.6);
  } else if (love_direction == LoveDirection::EMERGENCE) {
    updateLight(ASCEND_LIGHT, colors::EMERGENCE_LIGHT, 0.6);
    updateLight(PROGRESS_LIGHT, colors::EMERGENCE_LIGHT, 0.6);
  } else { // LoveDirection::NEW
    updateLight(ASCEND_LIGHT, colors::EMERGENCE_LIGHT, 0.6);
    updateLight(PROGRESS_LIGHT, colors::WOMB_LIGHT, 0.6f);
  }

}

void Circle::postProcessAlways(const ProcessArgs &args) {
  if (_light_divider.process()) {
    updateLights(args);
  }
}

void Circle::addChannel(int channel_i) {
  _engines[channel_i] = new kokopellivcv::dsp::circle::Engine();
  _engines[channel_i]->song.sample_time = _sampleTime;
}

void Circle::removeChannel(int channel_i) {
  delete _engines[channel_i];
  _engines[channel_i] = nullptr;
}

Model *modelCircle = rack::createModel<Circle, CircleWidget>("KokopelliInterfaces-Circle");
