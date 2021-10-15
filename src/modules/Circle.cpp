#include "Circle.hpp"
#include "CircleWidget.hpp"

Circle::Circle() {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  configParam(SELECT_PARAM, -INFINITY, INFINITY, 0.f, "Select");
  configParam(SELECT_MODE_PARAM, 0.f, 1.f, 0.f, "Select Mode");
  configParam(SELECT_FUNCTION_PARAM, 0.f, 1.f, 0.f, "Select Function");
  configParam(LOOP_PARAM, 0.f, 1.f, 0.f, "Loop");
  configParam(PREV_PARAM, 0.f, 1.f, 0.f, "Prev");
  configParam(NEXT_PARAM, 0.f, 1.f, 0.f, "Next");
  configParam(NEW_LOVE_PARAM, 0.f, 1.f, 0.f, "New Love");

  setBaseModelPredicate([](Model *m) { return m == modelSignal; });
  _light_divider.setDivision(512);
  _button_divider.setDivision(4);

  _select_function_button.param = &params[SELECT_FUNCTION_PARAM];
  _select_mode_button.param = &params[SELECT_MODE_PARAM];

  _prev_button.param = &params[PREV_PARAM];
  _next_button.param = &params[NEXT_PARAM];
  _loop_button.param = &params[LOOP_PARAM];
}

void Circle::sampleRateChange() {
  _sampleTime = APP->engine->getSampleTime();
  for (int c = 0; c < channels(); c++) {
    _engines[c]->_sample_time = _sampleTime;
  }
}

void Circle::processButtons() {
  float sampleTime = _sampleTime * _button_divider.division;

    kokopellivcv::dsp::LongPressButton::Event _select_function_event = _select_function_button.process(sampleTime);
    kokopellivcv::dsp::LongPressButton::Event _select_mode_event = _select_mode_button.process(sampleTime);
    kokopellivcv::dsp::LongPressButton::Event _prev_event = _prev_button.process(sampleTime);
    kokopellivcv::dsp::LongPressButton::Event _next_event = _next_button.process(sampleTime);
    kokopellivcv::dsp::LongPressButton::Event _loop_event = _loop_button.process(sampleTime);

  for (int c = 0; c < channels(); c++) {
    kokopellivcv::dsp::circle::Engine *e = _engines[c];

    switch (_select_function_event) {
    case kokopellivcv::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopellivcv::dsp::LongPressButton::SHORT_PRESS:
      if (e->_new_member_active) {
        e->_select_new_members = !e->_select_new_members;
      } else {
        e->toggleSelectMember(e->_active_member_i);
      }
      break;
    case kokopellivcv::dsp::LongPressButton::LONG_PRESS:
      if (e->isSelected(e->_active_member_i)) {
        if (e->_selected_members_idx.size() == 1) {
          e->_selected_members_idx = e->_saved_selected_members_idx;
        } else {
          e->_saved_selected_members_idx = e->_selected_members_idx;
          e->soloSelectMember(e->_active_member_i);
        }
      } else {
        if (e->_selected_members_idx.size() == 1) {
          e->selectRange(e->_selected_members_idx[0], e->_active_member_i);
        } else {
          e->selectRange(0, e->_active_member_i);
        }
      }
      break;
    }

    switch (_select_mode_event) {
    case kokopellivcv::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopellivcv::dsp::LongPressButton::SHORT_PRESS:
      break;
    case kokopellivcv::dsp::LongPressButton::LONG_PRESS:
      e->deleteSelection();
      break;
    }

    switch (_prev_event) {
    case kokopellivcv::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopellivcv::dsp::LongPressButton::SHORT_PRESS:
      e->prevMember();
      break;
    case kokopellivcv::dsp::LongPressButton::LONG_PRESS:
      e->undo();
      break;
    }

    switch (_next_event) {
    case kokopellivcv::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopellivcv::dsp::LongPressButton::SHORT_PRESS:
      e->nextMember();
      break;
    case kokopellivcv::dsp::LongPressButton::LONG_PRESS:
      // e->skipToActiveMember();
      if (0 < e->_timeline.members.size()) {
        e->_timeline.members[e->_active_member_i]->_loop = !e->_timeline.members[e->_active_member_i]->_loop;
      }
      break;
    }

    switch (_loop_event) {
    case kokopellivcv::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopellivcv::dsp::LongPressButton::SHORT_PRESS:
      e->loop();
      break;
    case kokopellivcv::dsp::LongPressButton::LONG_PRESS:
      e->loopLongPress();
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
      kokopellivcv::dsp::circle::Engine *e = _engines[c];

      bool increment = 0.f < d_select;
      if (increment) {
        if ((int) e->_timeline.members.size()-1 <= (int)e->_active_member_i) {
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
    kokopellivcv::dsp::circle::Engine *e = _engines[channel_i];
    float new_love = params[NEW_LOVE_PARAM].getValue();
    if (inputs[NEW_LOVE_INPUT].isConnected()) {
      new_love *= rack::clamp(inputs[NEW_LOVE_INPUT].getPolyVoltage(channel_i) / 10.f, 0.f, 1.0f);
    }

    // taking to the power of 2 gives a more intuitive curve
    new_love = pow(new_love, 2);
    e->_record_params.new_love = new_love;

    e->_use_ext_phase = inputs[PHASE_INPUT].isConnected();

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

  e->_record_params.in = _from_signal->signal[channel_i];
  e->step();
  _to_signal->signal[channel_i] = e->read();
  _to_signal->sel_signal[channel_i] = e->readSelection();
}

void Circle::updateLights(const ProcessArgs &args) {
  if (!baseConnected()) {
    return;
  }

  kokopellivcv::dsp::circle::Engine *default_e = _engines[0];

  lights[SELECT_FUNCTION_LIGHT + 0].value = 0.f;
  lights[SELECT_FUNCTION_LIGHT + 1].value = 0.f;
  if (default_e->_new_member_active || default_e->isRecording()) {
    if (default_e->_select_new_members) {
      lights[SELECT_FUNCTION_LIGHT + 1].value = 1.f;
      if (default_e->isSolo(default_e->_timeline.members.size()-1)) {
        lights[SELECT_FUNCTION_LIGHT + 0].value = 1.f;
      }
    }
  } else if (default_e->isSelected(default_e->_active_member_i)) {
    lights[SELECT_FUNCTION_LIGHT + 1].value = 1.f;
    if (default_e->_selected_members_idx.size() == 1) {
      lights[SELECT_FUNCTION_LIGHT + 0].value = 1.f;
    }
  }

  bool poly_record = (inputs[NEW_LOVE_INPUT].isConnected() && 1 < inputs[NEW_LOVE_INPUT].getChannels());

  kokopellivcv::dsp::circle::LoopMode displayed_loop_mode = default_e->_loop_mode;
  kokopellivcv::dsp::circle::RecordParams displayed_record_params = default_e->_record_params;

  float displayed_phase = default_e->_timeline_position.phase;

  float sel_signal_out_sum = 0.f;
  float signal_in_sum = 0.f;
  bool record_active = false;
  for (int c = 0; c < channels(); c++) {
    signal_in_sum += _from_signal->signal[c];
    sel_signal_out_sum += _to_signal->sel_signal[c];
    if (record_active) {
      displayed_loop_mode = _engines[c]->_loop_mode;
      displayed_record_params = _engines[c]->_record_params;
      displayed_phase = _engines[c]->_timeline_position.phase;
    }
  }

  float active_member_signal_out_sum = default_e->readActiveMember();
  active_member_signal_out_sum = rack::clamp(active_member_signal_out_sum, 0.f, 1.f);
  signal_in_sum = rack::clamp(signal_in_sum, 0.f, 1.f);
  sel_signal_out_sum = rack::clamp(sel_signal_out_sum, 0.f, 1.f);

  if (displayed_record_params.active()) {
    sel_signal_out_sum = sel_signal_out_sum * (1 - displayed_record_params.new_love);
    lights[EMERSIGN_LIGHT + 1].setSmoothBrightness(sel_signal_out_sum, _sampleTime * _light_divider.getDivision());
    int light_colour = poly_record ? 2 : 0;
    lights[EMERSIGN_LIGHT + light_colour].setSmoothBrightness(signal_in_sum, _sampleTime * _light_divider.getDivision());
  } else {
    lights[EMERSIGN_LIGHT + 0].value = 0.f;
    lights[EMERSIGN_LIGHT + 1].setSmoothBrightness(active_member_signal_out_sum, _sampleTime * _light_divider.getDivision());
    lights[EMERSIGN_LIGHT + 2].value = 0.f;
  }

  setLight(PREV_LIGHT, 0.f, 1.f, 0.f);
  setLight(NEXT_LIGHT, 0.f, 1.f, 0.f);

  if (default_e->isRecording()) {
    setLight(LOOP_MODE_LIGHT, 1.f, 1.f, 1.f);
  } else {
    switch (displayed_loop_mode) {
    case kokopellivcv::dsp::circle::LoopMode::None:
      setLight(LOOP_MODE_LIGHT, 1.f, 0.f, 0.f);
      break;
    case kokopellivcv::dsp::circle::LoopMode::Group:
      setLight(LOOP_MODE_LIGHT, 1.f, 1.f, 0.f);
      break;
    case kokopellivcv::dsp::circle::LoopMode::Member:
      setLight(LOOP_MODE_LIGHT, 0.f, 0.f, 1.f);
      break;
    }
  }

  bool poly_phase = (inputs[PHASE_INPUT].isConnected() && 1 < inputs[PHASE_INPUT].getChannels());
  if (poly_phase) {
    lights[PHASE_LIGHT + 1].value = 0.f;
    lights[PHASE_LIGHT + 2].setSmoothBrightness(displayed_phase, _sampleTime * _light_divider.getDivision());
  } else {
    lights[PHASE_LIGHT + 1].setSmoothBrightness(displayed_phase, _sampleTime * _light_divider.getDivision());
    lights[PHASE_LIGHT + 2].value = 0.f;
  }
}

void Circle::setLight(int light_n, float r, float g, float b) {
  lights[light_n + 0].value = r;
  lights[light_n + 1].value = g;
  lights[light_n + 2].value = b;
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

Model *modelCircle = rack::createModel<Circle, CircleWidget>("KokopelliVcv-Circle");
