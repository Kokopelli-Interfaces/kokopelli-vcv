#include "Circle.hpp"
#include "CircleWidget.hpp"

Circle::Circle() {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  configParam(SELECT_PARAM, -INFINITY, INFINITY, 0.f, "Select");
  configParam(SELECT_MODE_PARAM, 0.f, 1.f, 0.f, "Select Mode");
  configParam(SELECT_FUNCTION_PARAM, 0.f, 1.f, 0.f, "Select Function");
  configParam(SKIP_BACK_PARAM, 0.f, 1.f, 0.f, "Skip Back");
  configParam(FIX_BOUNDS_PARAM, 0.f, 1.f, 0.f, "Fix Recording Boundaries");
  configParam(RECORD_ON_INNER_CIRCLE_PARAM, 0.f, 1.f, 0.f, "Record On Inner Circle");
  configParam(RECORD_PARAM, 0.f, 1.f, 0.f, "Record Strength");

  setBaseModelPredicate([](Model *m) { return m == modelSignal; });
  _light_divider.setDivision(512);
  _button_divider.setDivision(4);

  _select_function_button.param = &params[SELECT_FUNCTION_PARAM];
  _select_mode_button.param = &params[SELECT_MODE_PARAM];

  _fix_bounds_button.param = &params[FIX_BOUNDS_PARAM];
  _record_on_inner_circle_button.param = &params[RECORD_ON_INNER_CIRCLE_PARAM];
  _skip_back_button.param = &params[SKIP_BACK_PARAM];
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
    kokopellivcv::dsp::LongPressButton::Event _fix_bounds_event = _fix_bounds_button.process(sampleTime);
    kokopellivcv::dsp::LongPressButton::Event _record_on_inner_circle_event = _record_on_inner_circle_button.process(sampleTime);
    kokopellivcv::dsp::LongPressButton::Event _skip_back_event = _skip_back_button.process(sampleTime);

  for (int c = 0; c < channels(); c++) {
    kokopellivcv::dsp::circle::Engine *e = _engines[c];

    switch (_select_function_event) {
    case kokopellivcv::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopellivcv::dsp::LongPressButton::SHORT_PRESS:
      if (e->_new_layer_active) {
        e->_select_new_layers = !e->_select_new_layers;
      } else {
        e->toggleSelectLayer(e->_active_layer_i);
      }
      break;
    case kokopellivcv::dsp::LongPressButton::LONG_PRESS:
      if (e->isSelected(e->_active_layer_i)) {
        if (e->_selected_layers_idx.size() == 1) {
          e->_selected_layers_idx = e->_saved_selected_layers_idx;
        } else {
          e->_saved_selected_layers_idx = e->_selected_layers_idx;
          e->soloSelectLayer(e->_active_layer_i);
        }
      } else {
        if (e->_selected_layers_idx.size() == 1) {
          e->selectRange(e->_selected_layers_idx[0], e->_active_layer_i);
        } else {
          e->selectRange(0, e->_active_layer_i);
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

    switch (_fix_bounds_event) {
    case kokopellivcv::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopellivcv::dsp::LongPressButton::SHORT_PRESS:
      if (!e->_record_params.fix_bounds) {
        e->setFixBounds(true);
      } else {
        e->setFixBounds(false);
      }
      break;
    case kokopellivcv::dsp::LongPressButton::LONG_PRESS:
      e->undo();
      break;
    }

    switch (_record_on_inner_circle_event) {
    case kokopellivcv::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopellivcv::dsp::LongPressButton::SHORT_PRESS:
      if (e->_record_params.record_on_inner_circle == false) {
        e->setRecordOnInnerLoop(true);
      } else {
        e->setRecordOnInnerLoop(false);
      }
      break;
    case kokopellivcv::dsp::LongPressButton::LONG_PRESS:
      // e->setCircleToActiveLayer();
      if (0 < e->_timeline.layers.size()) {
        e->_timeline.layers[e->_active_layer_i]->_loop = !e->_timeline.layers[e->_active_layer_i]->_loop;
      }
      break;
    }

    switch (_skip_back_event) {
    case kokopellivcv::dsp::LongPressButton::NO_PRESS:
      break;
    case kokopellivcv::dsp::LongPressButton::SHORT_PRESS:
      if (e->_skip_back == true) {
        e->setSkipBack(false);
      } else {
        e->setSkipBack(true);
      }
      break;
    case kokopellivcv::dsp::LongPressButton::LONG_PRESS:
      e->setCircleToActiveLayer();
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
        if ((int) e->_timeline.layers.size()-1 <= (int)e->_active_layer_i) {
          e->_new_layer_active = true;
        } else {
          e->_active_layer_i++;
        }
      } else {
        if (e->_new_layer_active) {
          e->_new_layer_active = false;
        } else if (0 < e->_active_layer_i) {
          e->_active_layer_i--;
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
    float record_strength = params[RECORD_PARAM].getValue();
    if (inputs[RECORD_INPUT].isConnected()) {
      record_strength *= rack::clamp(inputs[RECORD_INPUT].getPolyVoltage(channel_i) / 10.f, 0.f, 1.0f);
    }

    // taking to the strength of 2 gives a more intuitive curve
    record_strength = pow(record_strength, 2);
    e->_record_params.strength = record_strength;

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
  // LIGHT + 0 = RED
  // LIGHT + 1 = GREEN
  // LIGHT + 2 = BLUE

  if (!baseConnected()) {
    return;
  }

  float signal_in_sum = 0.f;

  kokopellivcv::dsp::circle::Engine *default_e = _engines[0];

  lights[SELECT_FUNCTION_LIGHT + 0].value = 0.f;
  lights[SELECT_FUNCTION_LIGHT + 1].value = 0.f;
  if (default_e->_new_layer_active || default_e->isRecording()) {
    if (default_e->_select_new_layers) {
      lights[SELECT_FUNCTION_LIGHT + 1].value = 1.f;
      if (default_e->isSolo(default_e->_timeline.layers.size()-1)) {
        lights[SELECT_FUNCTION_LIGHT + 0].value = 1.f;
      }
    }
  } else if (default_e->isSelected(default_e->_active_layer_i)) {
    lights[SELECT_FUNCTION_LIGHT + 1].value = 1.f;
    if (default_e->_selected_layers_idx.size() == 1) {
      lights[SELECT_FUNCTION_LIGHT + 0].value = 1.f;
    }
  }

  bool poly_record = (inputs[RECORD_INPUT].isConnected() && 1 < inputs[RECORD_INPUT].getChannels());

  bool displayed_skip_back = default_e->_skip_back;
  RecordParams displayed_record_params = default_e->_record_params;
  float displayed_phase = default_e->_timeline_position.phase;

  float active_layer_signal_out_sum = default_e->readActiveLayer();
  float sel_signal_out_sum = 0.f;

  bool record_active = false;
  for (int c = 0; c < channels(); c++) {
    signal_in_sum += _from_signal->signal[c];
    sel_signal_out_sum += _to_signal->sel_signal[c];
    if (record_active) {
      displayed_skip_back = _engines[c]->_skip_back;
      displayed_record_params = _engines[c]->_record_params;
      displayed_phase = _engines[c]->_timeline_position.phase;
    }
  }

  signal_in_sum = rack::clamp(signal_in_sum, 0.f, 1.f);
  active_layer_signal_out_sum = rack::clamp(active_layer_signal_out_sum, 0.f, 1.f);
  sel_signal_out_sum = rack::clamp(sel_signal_out_sum, 0.f, 1.f);

  if (displayed_record_params.active()) {
    sel_signal_out_sum = sel_signal_out_sum * (1 - displayed_record_params.strength);
    lights[RECORD_LIGHT + 1].setSmoothBrightness(sel_signal_out_sum, _sampleTime * _light_divider.getDivision());
    int light_colour = poly_record ? 2 : 0;
    lights[RECORD_LIGHT + light_colour].setSmoothBrightness(signal_in_sum, _sampleTime * _light_divider.getDivision());
  } else {
    lights[RECORD_LIGHT + 0].value = 0.f;
    lights[RECORD_LIGHT + 1].setSmoothBrightness(active_layer_signal_out_sum, _sampleTime * _light_divider.getDivision());
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

  lights[FIX_BOUNDS_LIGHT + 0].value = !displayed_record_params.fix_bounds;
  lights[FIX_BOUNDS_LIGHT + 1].value = displayed_record_params.fix_bounds;

  lights[RECORD_ON_INNER_CIRCLE_LIGHT + 0].value = !displayed_record_params.record_on_inner_circle;
  lights[RECORD_ON_INNER_CIRCLE_LIGHT + 1].value = displayed_record_params.record_on_inner_circle;

  if (default_e->isRecording()) {
    lights[SKIP_BACK_LIGHT + 0].value = 1.f;
    lights[SKIP_BACK_LIGHT + 1].value = 1.f;
    lights[SKIP_BACK_LIGHT + 2].value = 1.f;
  } else {
    lights[SKIP_BACK_LIGHT + 0].value = !displayed_skip_back;
    lights[SKIP_BACK_LIGHT + 1].value = displayed_skip_back;
    lights[SKIP_BACK_LIGHT + 2].value = 0.f;
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

Model *modelCircle = rack::createModel<Circle, CircleWidget>("KokopelliVcv-Circle");
