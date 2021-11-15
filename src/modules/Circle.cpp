#include "Circle.hpp"
#include "CircleWidget.hpp"

Circle::Circle() {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  configParam(TUNE_PARAM, 0.f, 1.f, 0.f, "Tune to Group Frequency");
  configParam(CYCLE_DIVINITY_PARAM, 0.f, 1.f, 0.f, "Cycle_Divinity");
  configParam(CYCLE_FORWARD_PARAM, 0.f, 1.f, 0.f, "Cycle_Forward");
  configParam(LOVE_PARAM, 0.f, 1.f, 0.f, "Love Direction");

  _light_divider.setDivision(512);
  _button_divider.setDivision(4);
  _light_blinker = new kokopellivcv::dsp::LightBlinker(&lights);

  _tune_button.param = &params[TUNE_PARAM];
  _cycle_forward_button.param = &params[CYCLE_FORWARD_PARAM];
  _cycle_divinity_button.param = &params[CYCLE_DIVINITY_PARAM];
}

Circle::~Circle() {
  delete _light_blinker;
}

// FIXME update gko phase oscillator
void Circle::sampleRateChange() {
  _sample_time = APP->engine->getSampleTime();
}

void Circle::processButtons(const ProcessArgs &args) {
  float sample_time = _sample_time * _button_divider.division;

  kokopellivcv::dsp::LongPressButton::Event _tune_event = _tune_button.process(sample_time);
  kokopellivcv::dsp::LongPressButton::Event _cycle_forward_event = _cycle_forward_button.process(sample_time);
  kokopellivcv::dsp::LongPressButton::Event _cycle_divinity_event = _cycle_divinity_button.process(sample_time);

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

    switch (_cycle_divinity_event) {
    case kokopellivcv::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopellivcv::dsp::LongPressButton::SHORT_PRESS:
      e->cycleDivinity();
      _light_blinker->blinkLight(CYCLE_DIVINITY_LIGHT);
      break;
    case kokopellivcv::dsp::LongPressButton::LONG_PRESS:
      e->descend();
      _light_blinker->blinkLight(CYCLE_DIVINITY_LIGHT); // TODO extra
      break;
    }

    switch (_cycle_forward_event) {
    case kokopellivcv::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopellivcv::dsp::LongPressButton::SHORT_PRESS:
      e->cycleForward();
      _light_blinker->blinkLight(CYCLE_FORWARD_LIGHT);
      break;
    case kokopellivcv::dsp::LongPressButton::LONG_PRESS:
      e->undo();
      _light_blinker->blinkLight(CYCLE_FORWARD_LIGHT); // TODO blink extra extra
      break;
    }
  }
}

void Circle::processAlways(const ProcessArgs &args) {
  outputs[PHASE_OUTPUT].setChannels(this->channels());
  outputs[SUN].setChannels(this->channels());
  outputs[ESTABLISHED_OUTPUT].setChannels(this->channels());

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

  e->_gko.use_ext_phase = inputs[PHASE_INPUT].isConnected();
  // FIXME ugh
  e->options = _options;
  e->_gko.love_resolution = _options.love_resolution;
  // e->_signal_type = _from_signal->signal_type;
}

int Circle::channels() {
  int input_channels = inputs[WOMB_INPUT].getChannels();
  if (_channels < input_channels) {
    for (int c = 0; c < _channels; c++) {
      _engines[c]->_gko.nextCycle(_engines[c]->_song, CycleEnd::DISCARD);
    }

    return input_channels;
  }

  return _channels;
}

void Circle::processChannel(const ProcessArgs& args, int channel_i) {
  kokopellivcv::dsp::circle::Engine *e = _engines[channel_i];

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

  LoveDirection love_direction = default_e->_gko._love_direction;
  if (default_e->_gko.tune_to_frequency_of_established) {
    updateLight(TUNE_LIGHT, colors::ESTABLISHED_LIGHT, default_e->_song.established_group->getPhase(default_e->_song.playhead));
  } else {
    long double playhead = default_e->_song.new_cycle->playhead;
    long double playhead_ms = rack::math::eucMod(playhead, 1.0f);
    updateLight(TUNE_LIGHT, colors::WOMB_LIGHT, playhead_ms);
  }

  if (love_direction == LoveDirection::ESTABLISHED) {
    updateLight(CYCLE_DIVINITY_LIGHT, colors::ESTABLISHED_LIGHT, 0.6f);
    updateLight(CYCLE_FORWARD_LIGHT, colors::EMERGENCE_LIGHT, 0.6);
  } else if (love_direction == LoveDirection::EMERGENCE) {
    updateLight(CYCLE_DIVINITY_LIGHT, colors::EMERGENCE_LIGHT, 0.6);
    updateLight(CYCLE_FORWARD_LIGHT, colors::EMERGENCE_LIGHT, 0.6);
  } else { // LoveDirection::NEW
    updateLight(CYCLE_DIVINITY_LIGHT, colors::EMERGENCE_LIGHT, 0.6);
    updateLight(CYCLE_FORWARD_LIGHT, colors::WOMB_LIGHT, 0.6f);
  }

}

void Circle::postProcessAlways(const ProcessArgs &args) {
  if (_light_divider.process()) {
    updateLights(args);
  }
}

void Circle::addChannel(int channel_i) {
  _engines[channel_i] = new kokopellivcv::dsp::circle::Engine();
  _engines[channel_i]->_gko.sample_time = _sample_time;
}

void Circle::removeChannel(int channel_i) {
  delete _engines[channel_i];
  _engines[channel_i] = nullptr;
}

Model *modelCircle = rack::createModel<Circle, CircleWidget>("KokopelliInterfaces-Circle");
