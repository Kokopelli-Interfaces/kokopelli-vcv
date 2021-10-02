#include "Circle.hpp"
#include "CircleWidget.hpp"

Circle::Circle() {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  configParam(SELECT_PARAM, -INFINITY, INFINITY, 0.f, "Select");
  configParam(SELECT_MODE_PARAM, 0.f, 1.f, 0.f, "Select Mode");
  configParam(SELECT_FUNCTION_PARAM, 0.f, 1.f, 0.f, "Select Function");
  configParam(REFLECT_PARAM, 0.f, 1.f, 0.f, "Reflect");
  configParam(PREV_MEMBER_PARAM, 0.f, 1.f, 0.f, "Focus Previous Circle Member");
  configParam(NEXT_MEMBER_PARAM, 0.f, 1.f, 0.f, "Focus Next Circle Member");
  configParam(LOVE_PARAM, 0.f, 1.f, 0.f, "Love");

  setBaseModelPredicate([](Model *m) { return m == modelSignal; });
  _light_divider.setDivision(512);
  _button_divider.setDivision(4);

  _select_function_button.param = &params[SELECT_FUNCTION_PARAM];
  _select_mode_button.param = &params[SELECT_MODE_PARAM];

  _previous_member_button.param = &params[PREV_MEMBER_PARAM];
  _next_member_button.param = &params[NEXT_MEMBER_PARAM];
  _reflect_button.param = &params[REFLECT_PARAM];
}

void Circle::sampleRateChange() {
  _sampleTime = APP->engine->getSampleTime();
  for (int c = 0; c < channels(); c++) {
    _engines[c]->interface->sample_time = _sampleTime;
  }
}

void Circle::processButtons() {
  float sampleTime = _sampleTime * _button_divider.division;

    kokopelli::dsp::LongPressButton::Event _select_function_event = _select_function_button.process(sampleTime);
    kokopelli::dsp::LongPressButton::Event _select_mode_event = _select_mode_button.process(sampleTime);
    kokopelli::dsp::LongPressButton::Event _previous_member_event = _previous_member_button.process(sampleTime);
    kokopelli::dsp::LongPressButton::Event _next_member_event = _next_member_button.process(sampleTime);
    kokopelli::dsp::LongPressButton::Event _reflect_event = _reflect_button.process(sampleTime);

  for (int c = 0; c < channels(); c++) {
    kokopelli::dsp::circle::Engine *e = _engines[c];

    switch (_select_function_event) {
    case kokopelli::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopelli::dsp::LongPressButton::SHORT_PRESS:
      break;
    case kokopelli::dsp::LongPressButton::LONG_PRESS:
      break;
    }

    switch (_select_mode_event) {
    case kokopelli::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopelli::dsp::LongPressButton::SHORT_PRESS:
      break;
    case kokopelli::dsp::LongPressButton::LONG_PRESS:
      e->deleteSelection();
      break;
    }

    switch (_previous_member_event) {
    case kokopelli::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopelli::dsp::LongPressButton::SHORT_PRESS:
      e->prevMember();
      break;
    case kokopelli::dsp::LongPressButton::LONG_PRESS:
      // TODO
      break;
    }

    switch (_next_member_event) {
    case kokopelli::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopelli::dsp::LongPressButton::SHORT_PRESS:
      e->nextMember();
      break;
    case kokopelli::dsp::LongPressButton::LONG_PRESS:
      break;
    }

    switch (_reflect_event) {
    case kokopelli::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopelli::dsp::LongPressButton::SHORT_PRESS:
      e->reflect();
      break;
    case kokopelli::dsp::LongPressButton::LONG_PRESS:
      e->undo();
      break;
    }
  }
}

void Circle::processSelect() {
  float select_value  = params[SELECT_PARAM].getValue();
  float d_select = select_value - _last_select_value;
  float value_per_rotation = 2.4f;
  int n_increments_per_rotation = 16;
  float select_change_threshold = value_per_rotation / n_increments_per_rotation;
  if (select_change_threshold < fabs(d_select)) {
    for (int c = 0; c < channels(); c++) {
      kokopelli::dsp::circle::Engine *e = _engines[c];

      bool increment = 0.f < d_select;
      if (increment) {
        if ((int) e->_cicle._members.size()-1 <= (int)e->_active_member_i) {
          e->_new_member_active = true;
        } else {
          e->_active_member_i++;
        }
      } else {
        if (e->_new_member_active) {
          e->_new_member_active = false;
        } else if (0 < e->_active_member_i) {
          e->_active_member_i--;
        }
      }
    }

    _last_select_value = select_value;
  }
}

void Circle::processAlways(const ProcessArgs &args) {
  if (baseConnected()) {
    _from_signal = fromBase();
    _to_signal = toBase();

    outputs[PHASE_OUTPUT].setChannels(this->channels());
  }

  if (_button_divider.process()) {
    processButtons();
  }
}

void Circle::modulate() {
  processSelect();
}

void Circle::modulateChannel(int channel_i) {
  if (baseConnected()) {
    kokopelli::dsp::circle::Engine *e = _engines[channel_i];
    float love = params[LOVE_PARAM].getValue();
    if (inputs[RECORD_INPUT].isConnected()) {
      love *= rack::clamp(inputs[RECORD_INPUT].getPolyVoltage(channel_i) / 10.f, 0.f, 1.0f);
    }

    e->interface->love = love;
    e->interface->use_ext_phase = inputs[PHASE_INPUT].isConnected();
    e->_options = _options;
    e->_signal_type = _from_signal->signal_type;
  }
}

// TODO base off max of Circle & sig
int Circle::channels() {
  if (baseConnected() && _from_signal) {
    int input_channels = _from_signal->channels;
    if (_channels < input_channels) {
      return input_channels;
    }
  }

  return _channels;
}

void Circle::processChannel(const ProcessArgs& args, int channel_i) {
  if (!baseConnected()) {
    return;
  }

  kokopelli::dsp::circle::Engine *e = _engines[channel_i];

  if (inputs[PHASE_INPUT].isConnected()) {
    float phase_in = inputs[PHASE_INPUT].getPolyVoltage(channel_i);
    if (_options.bipolar_phase_input) {
      phase_in += 5.0f;
    }

    e->interface->ext_phase = rack::clamp(phase_in / 10, 0.f, 1.0f);
  }

  if (outputs[PHASE_OUTPUT].isConnected()) {
    outputs[PHASE_OUTPUT].setVoltage(e->_phase * 10, channel_i);
  }

  e->interface->in = _from_signal->signal[channel_i];
  e->step();
  _to_signal->signal[channel_i] = e->read();
  _to_signal->sel_signal[channel_i] = e->readSelection();
}

void Circle::updateLights(const ProcessArgs &args) {
  // LIGHT + 0 = RED
  // LIGHT + 1 = GREEN
  // LIGHT + 2 = BLUE

  if (!baseConnected()) {
    return;
  }

  float signal_in_sum = 0.f;

  kokopelli::dsp::circle::Engine *default_e = _engines[0];

  lights[SELECT_FUNCTION_LIGHT + 0].value = 0.f;
  lights[SELECT_FUNCTION_LIGHT + 1].value = 0.f;
  if (default_e->_new_member_active || default_e->interface->isLoving()) {
    if (default_e->_select_new_members) {
      lights[SELECT_FUNCTION_LIGHT + 1].value = 1.f;
      if (default_e->isSolo(default_e->_cicle._members.size()-1)) {
        lights[SELECT_FUNCTION_LIGHT + 0].value = 1.f;
      }
    }
  } else if (default_e->isSelected(default_e->_active_member_i)) {
    lights[SELECT_FUNCTION_LIGHT + 1].value = 1.f;
    if (default_e->_selected_members_idx.size() == 1) {
      lights[SELECT_FUNCTION_LIGHT + 0].value = 1.f;
    }
  }

  bool poly_record = (inputs[RECORD_INPUT].isConnected() && 1 < inputs[RECORD_INPUT].getChannels());

  Interface* displayed_interface = default_e->interface;
  float displayed_phase = default_e->_phase;

  float active_member_signal_out_sum = default_e->readActiveMember();
  float sel_signal_out_sum = 0.f;

  bool record_active = false;
  for (int c = 0; c < channels(); c++) {
    signal_in_sum += _from_signal->signal[c];
    sel_signal_out_sum += _to_signal->sel_signal[c];
    if (record_active) {
      displayed_interface = _engines[c]->interface;
      displayed_phase = _engines[c]->_phase;
    }
  }

  signal_in_sum = rack::clamp(signal_in_sum, 0.f, 1.f);
  active_member_signal_out_sum = rack::clamp(active_member_signal_out_sum, 0.f, 1.f);
  sel_signal_out_sum = rack::clamp(sel_signal_out_sum, 0.f, 1.f);

  if (displayed_interface->isLoving()) {
    sel_signal_out_sum = sel_signal_out_sum * (1 - displayed_interface->love);
    lights[RECORD_LIGHT + 1].setSmoothBrightness(sel_signal_out_sum, _sampleTime * _light_divider.getDivision());
    int light_colour = poly_record ? 2 : 0;
    lights[RECORD_LIGHT + light_colour].setSmoothBrightness(signal_in_sum, _sampleTime * _light_divider.getDivision());
  } else {
    lights[RECORD_LIGHT + 0].value = 0.f;
    lights[RECORD_LIGHT + 1].setSmoothBrightness(active_member_signal_out_sum, _sampleTime * _light_divider.getDivision());
    lights[RECORD_LIGHT + 2].value = 0.f;
  }

  bool poly_phase = (inputs[PHASE_INPUT].isConnected() && 1 < inputs[PHASE_INPUT].getChannels());
  if (poly_phase) {
    lights[PHASE_LIGHT + 1].value = 0.f;
    lights[PHASE_LIGHT + 2].setSmoothBrightness(displayed_phase, _sampleTime * _light_divider.getDivision());
  } else {
    lights[PHASE_LIGHT + 1].setSmoothBrightness(displayed_phase, _sampleTime * _light_divider.getDivision());
    lights[PHASE_LIGHT + 2].value = 0.f;
  }

  // lights[PREVIOUS_MEMBER_LIGHT + 0].value = !displayed_interface.previous_member;
  // lights[PREVIOUS_MEMBER_LIGHT + 1].value = displayed_interface.previous_member;
  // lights[NEXT_MEMBER_LIGHT + 0].value = !displayed_interface.next_member;
  // lights[NEXT_MEMBER_LIGHT + 1].value = displayed_interface.next_member;

  if (default_e->interface->isLoving()) {
    lights[REFLECT_LIGHT + 0].value = 1.f;
    lights[REFLECT_LIGHT + 1].value = 1.f;
    lights[REFLECT_LIGHT + 2].value = 1.f;
  } else { // TODO depend on mode
    lights[REFLECT_LIGHT + 0].value = 0.0f;
    lights[REFLECT_LIGHT + 1].value = 1.0f;
    lights[REFLECT_LIGHT + 2].value = 0.f;
  }
}

void Circle::postProcessAlways(const ProcessArgs &args) {
  if (_light_divider.process()) {
    updateLights(args);
  }
}

void Circle::addChannel(int channel_i) {
  _engines[channel_i] = new kokopelli::dsp::circle::Engine();
  _engines[channel_i]->interface->sample_time = _sampleTime;
}

void Circle::removeChannel(int channel_i) {
  delete _engines[channel_i];
  _engines[channel_i] = nullptr;
}

Model *modelCircle = rack::createModel<Circle, CircleWidget>("Kokopelli-Circle");
