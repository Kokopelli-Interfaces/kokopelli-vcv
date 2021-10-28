#include "Circle.hpp"
#include "CircleWidget.hpp"

Circle::Circle() {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  configParam(MODE_PARAM, 0.f, 1.f, 0.f, "Mode");
  configParam(PREVIOUS_PARAM, 0.f, 1.f, 0.f, "Focus Previous");
  configParam(NEXT_PARAM, 0.f, 1.f, 0.f, "Focus Next");
  configParam(LOVE_PARAM, 0.f, 1.f, 0.f, "Love");

  _light_divider.setDivision(512);
  _button_divider.setDivision(4);
  _light_blinker = new kokopellivcv::dsp::LightBlinker(&lights);

  _mode_button.param = &params[MODE_PARAM];
  _next_button.param = &params[NEXT_PARAM];
  _previous_button.param = &params[PREVIOUS_PARAM];
}

Circle::~Circle() {
  delete _light_blinker;
}

void Circle::sampleRateChange() {
  _sampleTime = APP->engine->getSampleTime();
}

void Circle::processButtons(const ProcessArgs &args) {
  float sampleTime = _sampleTime * _button_divider.division;

  kokopellivcv::dsp::LongPressButton::Event _mode_event = _mode_button.process(sampleTime);
  kokopellivcv::dsp::LongPressButton::Event _next_event = _next_button.process(sampleTime);
  kokopellivcv::dsp::LongPressButton::Event _prev_event = _previous_button.process(sampleTime);

  for (int c = 0; c < channels(); c++) {
    kokopellivcv::dsp::circle::Engine *e = _engines[c];

    switch (_mode_event) {
    case kokopellivcv::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopellivcv::dsp::LongPressButton::SHORT_PRESS:
      e->toggleFixBounds();
      updateLights(args);
      _light_blinker->blinkLight(MODE_LIGHT);
      break;
    case kokopellivcv::dsp::LongPressButton::LONG_PRESS:
      // e->toggleMemberMode();
      break;
    }

    switch (_prev_event) {
    case kokopellivcv::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopellivcv::dsp::LongPressButton::SHORT_PRESS:
      e->prev();
      _light_blinker->blinkLight(PREVIOUS_LIGHT);
      break;
    case kokopellivcv::dsp::LongPressButton::LONG_PRESS:
      e->forget();
      break;
    }

    switch (_next_event) {
    case kokopellivcv::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopellivcv::dsp::LongPressButton::SHORT_PRESS:
      e->next();
      _light_blinker->blinkLight(NEXT_LIGHT);
      break;
    case kokopellivcv::dsp::LongPressButton::LONG_PRESS:
      // e->skipToFocusedMember();
      e->deleteSelection();
      e->skipToFocusedMember();
      break;
    }
  }
}

void Circle::processAlways(const ProcessArgs &args) {
  outputs[PHASE_OUTPUT].setChannels(this->channels());

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
  e->_record_params.love = love;

  e->_use_ext_phase = inputs[PHASE_INPUT].isConnected();
  e->_options = _options;
  e->_timeline._attenuation_resolution = _attenuation_resolution;
  // e->_signal_type = _from_signal->signal_type;
}

// TODO base off max of Circle & sig
int Circle::channels() {
  int input_channels = inputs[MEMBER_INPUT].getChannels();
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

    e->_ext_phase = rack::clamp(phase_in / 10, 0.f, 1.0f);
  }

  if (outputs[PHASE_OUTPUT].isConnected()) {
    outputs[PHASE_OUTPUT].setVoltage(e->_timeline_position.phase * 10, channel_i);
  }

  if (inputs[MEMBER_INPUT].isConnected()) {
    e->_record_params.in = inputs[MEMBER_INPUT].getPolyVoltage(channel_i);
  } else {
    e->_record_params.in = 0.f;
  }

  e->step();

  if (outputs[CIRCLE_OUTPUT].isConnected()) {
    outputs[CIRCLE_OUTPUT].setVoltage(e->read(), channel_i);
  }

  if (outputs[GROUP_OUTPUT].isConnected()) {
    outputs[GROUP_OUTPUT].setVoltage(e->readSelection(), channel_i);
  }
}

void Circle::updateLight(int light, float r, float g, float b) {
  if (_light_blinker->_op && _light_blinker->_light_i == light) {
    return;
  }

  lights[light + 0].value = r / 2.f;
  lights[light + 1].value = g / 2.f;
  lights[light + 2].value = b / 2.f;
}

void Circle::updateLights(const ProcessArgs &args) {
  _light_blinker->step();

  kokopellivcv::dsp::circle::Engine *default_e = _engines[0];

  bool poly_record = (inputs[LOVE_INPUT].isConnected() && 1 < inputs[LOVE_INPUT].getChannels());

  RecordParams displayed_record_params = default_e->_record_params;

  float focused_member_signal_out_sum = default_e->readFocusedMember();

  float member_in_sum = 0.f;
  bool record_active = false;
  for (int c = 0; c < channels(); c++) {
    member_in_sum += inputs[MEMBER_INPUT].getPolyVoltage(c);
    if (record_active) {
      displayed_record_params = _engines[c]->_record_params;
    }
  }

  member_in_sum = rack::clamp(member_in_sum, 0.f, 1.f);
  focused_member_signal_out_sum = rack::clamp(focused_member_signal_out_sum, 0.f, 1.f);

  // NOTE fix
  if (default_e->isRecording()) {
    lights[EMERSIGN_LIGHT + 1].value = 0.f;
    int light_colour = poly_record ? 2 : 0;
    lights[EMERSIGN_LIGHT + light_colour].setSmoothBrightness(member_in_sum, _sampleTime * _light_divider.getDivision());
  } else {
    lights[EMERSIGN_LIGHT + 0].value = 0.f;
    lights[EMERSIGN_LIGHT + 1].setSmoothBrightness(focused_member_signal_out_sum, _sampleTime * _light_divider.getDivision());
    lights[EMERSIGN_LIGHT + 2].value = 0.f;
  }

  bool fix_bounds = displayed_record_params.fix_bounds;
  if (default_e->isRecording()) {
    updateLight(MODE_LIGHT, !fix_bounds, fix_bounds, default_e->_member_mode);
    updateLight(PREVIOUS_LIGHT, (float)!fix_bounds * .2f, 1.f, (float)fix_bounds * .2f);
    updateLight(NEXT_LIGHT, (float)!fix_bounds * .2f, 1.f, (float)fix_bounds * .2f);
  } else {
    updateLight(MODE_LIGHT, !fix_bounds, fix_bounds, default_e->_member_mode);
    updateLight(PREVIOUS_LIGHT, 0.f, 1.f,  0.f);
    updateLight(NEXT_LIGHT, 0.f, 1.0,  0.f);
  }
}

void Circle::postProcessAlways(const ProcessArgs &args) {
  if (_light_divider.process()) {
    updateLights(args);
  }
}

void Circle::addChannel(int channel_i) {
  _engines[channel_i] = new kokopellivcv::dsp::circle::Engine();
  _engines[channel_i]->_sample_time = _sampleTime;
}

void Circle::removeChannel(int channel_i) {
  delete _engines[channel_i];
  _engines[channel_i] = nullptr;
}

Model *modelCircle = rack::createModel<Circle, CircleWidget>("KokopelliInterfaces-Circle");
