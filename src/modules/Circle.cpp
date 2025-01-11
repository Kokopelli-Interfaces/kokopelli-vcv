#include "Circle.hpp"
#include "CircleWidget.hpp"

Circle::Circle() {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  configParam(AUDITION_PARAM, 0.f, 2.f, 1.f, "Solo (Observed Song <-> Band Input)");
  configParam(DIVINITY_PARAM, 0.f, 1.f, 0.f, "Change Observed Song (Long Press to Ascend) (Press While Change Active to Capture Window)");
  configParam(CYCLE_PARAM, 0.f, 1.f, 0.f, "Enter Group/Refresh Capture (Long Press to Undo) (Press While Change Fully Active to Remove Observered Song)");
  configParam(LOVE_PARAM, 0.f, 1.f, 0.f, "Change");

  configInput(BAND_INPUT, "Band");
  configInput(PHASE_INPUT, "Phase");
  configOutput(OBSERVER_OUTPUT, "Observed Song");
  configOutput(OBSERVER_PHASE_OUTPUT, "Observed Song Phase");
  configOutput(OBSERVER_BEAT_PHASE_OUTPUT, "Beat Phase");
  configOutput(SUN, "Total Song");

  configBypass(BAND_INPUT, SUN);

  _light_divider.setDivision(512);
  _button_divider.setDivision(4);
  _light_blinker = new kokopellivcv::dsp::LightBlinker(&lights);

  _cycle_forward_button.param = &params[CYCLE_PARAM];
  _cycle_divinity_button.param = &params[DIVINITY_PARAM];

  CIRCLES.push_back(this);
  // printf("CIRCLES size is %ld\n", CIRCLES.size());
}

Circle::~Circle() {
  // FIXME memory SPILLAGE
  delete _light_blinker;

  for (unsigned int i = 0; i < CIRCLES.size(); i++) {
    if (CIRCLES[i] == this) {
      CIRCLES.erase(CIRCLES.begin()+i);
    }
  }
}

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
  _options.poly_input_phase_mode = 1 < inputs[PHASE_INPUT].getChannels();

  return;
}

void Circle::modulateChannel(int channel_i) {
  kokopellivcv::dsp::circle::Engine *e = _engines[channel_i];
  float love = params[LOVE_PARAM].getValue();

  // more intuitive curve
  love = pow(love, 2);
  e->inputs.love = love;

  e->options = _options;

  e->_gko.use_ext_phase = inputs[PHASE_INPUT].isConnected();
}

int Circle::channels() {
  int input_channels = inputs[BAND_INPUT].getChannels();
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

  if (inputs[BAND_INPUT].isConnected()) {
    e->inputs.in = inputs[BAND_INPUT].getPolyVoltage(channel_i);
  } else {
    e->inputs.in = 0.f;
  }

  if (inputs[PHASE_INPUT].isConnected()) {
    float phase_in = 0.f;
    if (_options.poly_input_phase_mode) {
      phase_in = inputs[PHASE_INPUT].getPolyVoltage(channel_i);
    } else {
      phase_in = inputs[PHASE_INPUT].getVoltage();
    }

    e->_gko.ext_phase = rack::clamp(phase_in / 10, 0.f, 1.0f);

    e->inputs.in = inputs[BAND_INPUT].getPolyVoltage(channel_i);
  }

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
    new_sum += inputs[BAND_INPUT].getPolyVoltage(c);
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
    updateLight(CYCLE_LIGHT, colors::BAND_LIGHT, light_strength);
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

  for (int c = 0; c < channels(); c++) {
    _engines[c]->channelStateReset();
  }
}

void Circle::removeChannel(int channel_i) {
  delete _engines[channel_i];
  _engines[channel_i] = nullptr;
}

Model *modelCircle = rack::createModel<Circle, CircleWidget>("KokopelliInterfaces-Circle");
