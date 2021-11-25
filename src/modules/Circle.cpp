#include "Circle.hpp"
#include "CircleWidget.hpp"

Circle::Circle() {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  configParam(AUDITION_PARAM, 0.f, 2.f, 1.f, "Solo Audio (Established or Creation)");
  configParam(DIVINITY_PARAM, 0.f, 1.f, 0.f, "Select");
  configParam(CYCLE_PARAM, 0.f, 1.f, 0.f, "Next");
  configParam(LOVE_PARAM, 0.f, 1.f, 0.f, "Love Direction (Established or Creation)");

  _light_divider.setDivision(512);
  _button_divider.setDivision(4);
  _light_blinker = new kokopellivcv::dsp::LightBlinker(&lights);

  _cycle_forward_button.param = &params[CYCLE_PARAM];
  _cycle_divinity_button.param = &params[DIVINITY_PARAM];
}

Circle::~Circle() {
  delete _light_blinker;
}

void Circle::updateLoveResolution() {
  for (int c = 0; c < channels(); c++) {
    kokopellivcv::dsp::circle::Engine *e = _engines[c];
    e->_gko.love_updater.updateLoveResolution(_options.love_resolution);
  }
}

// FIXME update gko phase oscillator
void Circle::sampleRateChange() {
  _sample_time = APP->engine->getSampleTime();
}

void Circle::processButtons(const ProcessArgs &args) {
  float sample_time = _sample_time * _button_divider.division;

  kokopellivcv::dsp::LongPressButton::Event _cycle_forward_event = _cycle_forward_button.process(sample_time);
  kokopellivcv::dsp::LongPressButton::Event _cycle_divinity_event = _cycle_divinity_button.process(sample_time);

  for (int c = 0; c < channels(); c++) {
    kokopellivcv::dsp::circle::Engine *e = _engines[c];

    switch (_cycle_divinity_event) {
    case kokopellivcv::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopellivcv::dsp::LongPressButton::SHORT_PRESS:
      e->cycleObservation();
      _light_blinker->blinkLight(DIVINITY_LIGHT);
      break;
    case kokopellivcv::dsp::LongPressButton::LONG_PRESS:
      e->ascend();
      _light_blinker->blinkLight(DIVINITY_LIGHT); // TODO extra
      break;
    }

    switch (_cycle_forward_event) {
    case kokopellivcv::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopellivcv::dsp::LongPressButton::SHORT_PRESS:
      e->cycleForward();
      _light_blinker->blinkLight(CYCLE_LIGHT);
      break;
    case kokopellivcv::dsp::LongPressButton::LONG_PRESS:
      e->undo();
      _light_blinker->blinkLight(CYCLE_LIGHT); // TODO blink extra extra
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


static inline float smoothValue(float current, float old) {
  float lambda = .1;
  return old + (current - old) * lambda;
}

void Circle::modulate() {
  _audition_position = smoothValue(params[AUDITION_PARAM].getValue(), _audition_position);
  return;
}

void Circle::modulateChannel(int channel_i) {
  kokopellivcv::dsp::circle::Engine *e = _engines[channel_i];
  float love = params[LOVE_PARAM].getValue();
  if (inputs[LOVE_INPUT].isConnected()) {
    love *= rack::clamp(inputs[LOVE_INPUT].getPolyVoltage(channel_i) / 10.f, 0.f, 1.0f);
  }

  // more intuitive curve
  love = pow(love, 2);
  e->inputs.love = love;

  e->_gko.use_ext_phase = inputs[PHASE_INPUT].isConnected();

  // FIXME ugh
  e->options = _options;
  e->_gko.love_updater.love_resolution = _options.love_resolution;
  // e->_signal_type = _from_signal->signal_type;
}

int Circle::channels() {
  int input_channels = inputs[WOMB_INPUT].getChannels();
  if (_channels < input_channels) {
    for (int c = 0; c < _channels; c++) {
      _engines[c]->channelStateReset();
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

  if (inputs[PHASE_INPUT].isConnected()) {
    float phase_in = inputs[PHASE_INPUT].getPolyVoltage(channel_i);
    e->_gko.ext_phase = rack::clamp(phase_in / 10, 0.f, 1.0f);
  }

  if (outputs[PHASE_OUTPUT].isConnected()) {
    float phase;
    if (_options.output_beat_phase) {
      phase = e->_song.established->getBeatPhase(e->_song.playhead);
    } else {
      phase = e->_song.established->getPhase(e->_song.playhead);
    }
    outputs[PHASE_OUTPUT].setVoltage(phase * 10, channel_i);
  }

  e->step();

  Outputs out = e->_song.out;
  if (outputs[SUN].isConnected()) {
    float sun_out = 0.f;
    if (1.f == _audition_position) {
      sun_out = out.sun;
    } else if (1.f < _audition_position) {
      float established_and_input = kokopellivcv::dsp::sum(out.attenuated_established, e->inputs.in, e->_signal_type);
      sun_out = rack::crossfade(out.sun, established_and_input, _audition_position - 1.f);
    } else {
      sun_out = rack::crossfade( e->inputs.in, out.sun, _audition_position);
    }

    outputs[SUN].setVoltage(sun_out, channel_i);
  }

  if (outputs[ESTABLISHED_OUTPUT].isConnected()) {
    outputs[ESTABLISHED_OUTPUT].setVoltage(out.established, channel_i);
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
  if (love_direction == LoveDirection::ESTABLISHED) {
    updateLight(DIVINITY_LIGHT, colors::ESTABLISHED_LIGHT, 0.6f);
    if (default_e->_gko.observer.checkIfInSubgroupMode()) {
      updateLight(CYCLE_LIGHT, colors::ESTABLISHED_LIGHT, 0.6);
    } else {
      updateLight(CYCLE_LIGHT, colors::EMERGENCE_LIGHT, 0.6);
    }
  } else if (love_direction == LoveDirection::EMERGENCE) {
    updateLight(DIVINITY_LIGHT, colors::EMERGENCE_LIGHT, 0.6);
    updateLight(CYCLE_LIGHT, colors::EMERGENCE_LIGHT, 0.6);
  } else { // LoveDirection::NEW
    updateLight(DIVINITY_LIGHT, colors::EMERGENCE_LIGHT, 0.6);
    updateLight(CYCLE_LIGHT, colors::WOMB_LIGHT, 0.6f);
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
