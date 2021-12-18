#include "Circle.hpp"
#include "CircleWidget.hpp"

Circle::Circle() {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  configParam(AUDITION_PARAM, 0.f, 2.f, 1.f, "Filter Sun Audio");
  configParam(DIVINITY_PARAM, 0.f, 1.f, 0.f, "Rotate Observer");
  configParam(CYCLE_PARAM, 0.f, 1.f, 0.f, "Next Movement");
  configParam(LOVE_PARAM, 0.f, 1.f, 0.f, "Love Direction (Sun <-(emergence)-> Moon)");

  configInput(WOMB_INPUT, "Moon (Creative Background/Yin/Orchestra/Feminine/Womb of Universe)");
  configOutput(OBSERVER_OUTPUT, "Observer (Soul/Consciousness)");
  configOutput(OBSERVER_PHASE_OUTPUT, "Observer Phase");
  configOutput(OBSERVER_BEAT_PHASE_OUTPUT, "Beat Phase");
  configOutput(SUN, "Sun (Emernet/Yang/Song/Masculine/Universe)");

  configBypass(WOMB_INPUT, SUN);

  _light_divider.setDivision(512);
  _button_divider.setDivision(4);
  _light_blinker = new kokopellivcv::dsp::LightBlinker(&lights);

  _cycle_forward_button.param = &params[CYCLE_PARAM];
  _cycle_divinity_button.param = &params[DIVINITY_PARAM];
}

Circle::~Circle() {
  delete _light_blinker;
}

// FIXME update gko phase oscillato
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
      _light_blinker->blinkLight(DIVINITY_LIGHT, 3.f);
      break;
    case kokopellivcv::dsp::LongPressButton::LONG_PRESS:
      e->ascend();
      _light_blinker->blinkLight(DIVINITY_LIGHT, 0.f); // TODO extra
      break;
    }

    switch (_cycle_forward_event) {
    case kokopellivcv::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopellivcv::dsp::LongPressButton::SHORT_PRESS:
      e->cycleForward();
      _light_blinker->blinkLight(CYCLE_LIGHT, 3.f);
      break;
    case kokopellivcv::dsp::LongPressButton::LONG_PRESS:
      e->undo();
      _light_blinker->blinkLight(CYCLE_LIGHT, 0.f); // TODO blink extra extra
      break;
    }
  }
}

void Circle::processAlways(const ProcessArgs &args) {
  outputs[OBSERVER_PHASE_OUTPUT].setChannels(this->channels());
  outputs[SUN].setChannels(this->channels());
  outputs[OBSERVER_OUTPUT].setChannels(this->channels());

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

  if (_love_resolution != _options.love_resolution) {
    _love_resolution = _options.love_resolution;
    for (int c = 0; c < channels(); c++) {
      kokopellivcv::dsp::circle::Engine *e = _engines[c];
      e->_gko.love_updater.updateLoveResolution(_love_resolution);
    }
  }

  // TODO FIXME
  if (_delay_shiftback != _options.delay_shiftback) {
    _delay_shiftback = _options.delay_shiftback;
    for (int c = 0; c < channels(); c++) {
      kokopellivcv::dsp::circle::Engine *e = _engines[c];
      Time delay_shiftback_time = _delay_shiftback * _sample_time;
      e->_gko.delay_shiftback = delay_shiftback_time;
    }
  }

  return;
}

void Circle::modulateChannel(int channel_i) {
  kokopellivcv::dsp::circle::Engine *e = _engines[channel_i];
  float love = params[LOVE_PARAM].getValue();

  // more intuitive curve
  love = pow(love, 2);
  e->inputs.love = love;

  // TODO AION
  // e->_gko.use_ext_phase = inputs[PHASE_INPUT].isConnected();

  e->_gko.include_moon_in_sun_output = _options.include_moon_in_sun_output;
  e->_gko.include_moon_in_observer_output = _options.include_moon_in_observer_output;

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

  // TODO get from AION
  // if (inputs[PHASE_INPUT].isConnected()) {
  //   float phase_in = inputs[PHASE_INPUT].getPolyVoltage(channel_i);
  //   e->_gko.ext_phase = rack::clamp(phase_in / 10, 0.f, 1.0f);
  // }

  if (outputs[OBSERVER_PHASE_OUTPUT].isConnected()) {
    float phase = e->_song.observed_sun->getPhase(e->_song.playhead);
    outputs[OBSERVER_PHASE_OUTPUT].setVoltage(phase * 10, channel_i);
  }

  if (outputs[OBSERVER_BEAT_PHASE_OUTPUT].isConnected()) {
    float phase = e->_song.observed_sun->getBeatPhase(e->_song.playhead);
    outputs[OBSERVER_BEAT_PHASE_OUTPUT].setVoltage(phase * 10, channel_i);
  }

  e->step();

  Outputs out = e->_song.out;
  if (outputs[SUN].isConnected()) {
    float sun_out = 0.f;
    if (_audition_position < 1.f) {
      float observed_sun_and_input = kokopellivcv::dsp::sum(out.attenuated_observed_sun, e->inputs.in, e->_signal_type);
      sun_out = rack::crossfade(observed_sun_and_input, out.sun, _audition_position);
    } else if (1.f == _audition_position) {
      sun_out = out.sun;
    } else if (1.f < _audition_position) {
      sun_out = rack::crossfade(out.sun, e->inputs.in, _audition_position - 1.f);
    }

    outputs[SUN].setVoltage(sun_out, channel_i);
  }

  if (outputs[OBSERVER_OUTPUT].isConnected()) {
    outputs[OBSERVER_OUTPUT].setVoltage(out.observed_sun, channel_i);
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
  float observed_sun_sum = 0.f;
  for (int c = 0; c < channels(); c++) {
    new_sum += inputs[WOMB_INPUT].getPolyVoltage(c);
    observed_sun_sum += outputs[OBSERVER_OUTPUT].getPolyVoltage(c);
  }
  new_sum = rack::clamp(new_sum, 0.f, 1.f);
  observed_sun_sum = rack::clamp(observed_sun_sum, 0.f, 1.f);
  observed_sun_sum = observed_sun_sum * (1.f - default_e->inputs.love);

  float light_strength = .3f;
  LoveDirection love_direction = default_e->_gko._love_direction;
  if (love_direction == LoveDirection::OBSERVED_SUN) {
    if (!default_e->_song.cycles.empty()) {
      updateLight(DIVINITY_LIGHT, colors::OBSERVED_SUN_LIGHT, light_strength);
    } else {
      updateLight(DIVINITY_LIGHT, colors::OBSERVED_SUN_LIGHT, 0.f);
    }

    if (default_e->_gko.observer.checkIfInSubgroupMode()) {
      if (default_e->_gko.observer.checkIfCanEnterFocusedSubgroup()) {
        updateLight(CYCLE_LIGHT, colors::OBSERVED_SUN_LIGHT, light_strength);
      } else {
        updateLight(CYCLE_LIGHT, colors::OBSERVED_SUN_LIGHT, 0.f);
      }
    } else {
      updateLight(CYCLE_LIGHT, colors::EMERGENCE_LIGHT, light_strength);
    }
  } else if (love_direction == LoveDirection::EMERGENCE) {
    updateLight(DIVINITY_LIGHT, colors::EMERGENCE_LIGHT, light_strength);
    updateLight(CYCLE_LIGHT, colors::EMERGENCE_LIGHT, light_strength);
  } else { // LoveDirection::NEW
    updateLight(DIVINITY_LIGHT, colors::EMERGENCE_LIGHT, light_strength);
    updateLight(CYCLE_LIGHT, colors::WOMB_LIGHT, light_strength);
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
