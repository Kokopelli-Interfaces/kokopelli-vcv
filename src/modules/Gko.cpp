#include "Gko.hpp"
#include "GkoWidget.hpp"

Gko::Gko() {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  configParam(SELECT_PARAM, -INFINITY, INFINITY, 0.f, "Select");
  configParam(SELECT_MODE_PARAM, 0.f, 1.f, 0.f, "Select Mode");
  configParam(SELECT_FUNCTION_PARAM, 0.f, 1.f, 0.f, "Select Function");
  configParam(READ_TIME_FRAME_PARAM, 0.f, 1.f, 0.f, "Read Time Frame");
  configParam(RECORD_MODE_PARAM, 0.f, 1.f, 0.f, "Record Mode");
  configParam(RECORD_TIME_FRAME_PARAM, 0.f, 1.f, 0.f, "Record Time Frame");
  configParam(RECORD_PARAM, 0.f, 1.f, 0.f, "Record Strength");

  setBaseModelPredicate([](Model *m) { return m == modelSignal; });
  _light_divider.setDivision(512);
  _button_divider.setDivision(4);

  _select_function_button.param = &params[SELECT_FUNCTION_PARAM];
  _select_mode_button.param = &params[SELECT_MODE_PARAM];

  _record_mode_button.param = &params[RECORD_MODE_PARAM];
  _record_time_frame_button.param = &params[RECORD_TIME_FRAME_PARAM];
  _read_time_frame_button.param = &params[READ_TIME_FRAME_PARAM];
}

void Gko::sampleRateChange() {
  _sampleTime = APP->engine->getSampleTime();
  for (int c = 0; c < channels(); c++) {
    _engines[c]->_sample_time = _sampleTime;
  }
}

void Gko::processButtons() {
  float sampleTime = _sampleTime * _button_divider.division;

    myrisa::dsp::LongPressButton::Event _select_function_event = _select_function_button.process(sampleTime);
    myrisa::dsp::LongPressButton::Event _select_mode_event = _select_mode_button.process(sampleTime);
    myrisa::dsp::LongPressButton::Event _record_mode_event = _record_mode_button.process(sampleTime);
    myrisa::dsp::LongPressButton::Event _record_time_frame_event = _record_time_frame_button.process(sampleTime);
    myrisa::dsp::LongPressButton::Event _read_time_frame_event = _read_time_frame_button.process(sampleTime);

  for (int c = 0; c < channels(); c++) {
    myrisa::dsp::gko::Engine *e = _engines[c];

    switch (_select_function_event) {
    case myrisa::dsp::LongPressButton::NO_PRESS:
      break;
    case myrisa::dsp::LongPressButton::SHORT_PRESS:
      if (e->_new_layer_active) {
        e->_select_new_layers = !e->_select_new_layers;
      } else {
        e->toggleSelectLayer(e->_active_layer_i);
      }
      break;
    case myrisa::dsp::LongPressButton::LONG_PRESS:
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
    case myrisa::dsp::LongPressButton::NO_PRESS:
      break;
    case myrisa::dsp::LongPressButton::SHORT_PRESS:
      break;
    case myrisa::dsp::LongPressButton::LONG_PRESS:
      e->deleteSelection();
      break;
    }

    switch (_record_mode_event) {
    case myrisa::dsp::LongPressButton::NO_PRESS:
      break;
    case myrisa::dsp::LongPressButton::SHORT_PRESS:
      if (e->_record_params.mode == RecordParams::Mode::DUB) {
        e->setRecordMode(RecordParams::Mode::EXTEND);
      } else {
        e->setRecordMode(RecordParams::Mode::DUB);
      }
      break;
    case myrisa::dsp::LongPressButton::LONG_PRESS:
      e->undo();
      break;
    }

    switch (_record_time_frame_event) {
    case myrisa::dsp::LongPressButton::NO_PRESS:
      break;
    case myrisa::dsp::LongPressButton::SHORT_PRESS:
      if (e->_record_params.time_frame == TimeFrame::CIRCLE) {
        e->setRecordTimeFrame(TimeFrame::TIME);
      } else {
        e->setRecordTimeFrame(TimeFrame::CIRCLE);
      }
      break;
    case myrisa::dsp::LongPressButton::LONG_PRESS:
      // e->setCircleToActiveLayer();
      if (0 < e->_timeline.layers.size()) {
        e->_timeline.layers[e->_active_layer_i]->_loop = !e->_timeline.layers[e->_active_layer_i]->_loop;
      }
      break;
    }

    switch (_read_time_frame_event) {
    case myrisa::dsp::LongPressButton::NO_PRESS:
      break;
    case myrisa::dsp::LongPressButton::SHORT_PRESS:
      if (e->_read_time_frame == TimeFrame::CIRCLE) {
        e->setReadTimeFrame(TimeFrame::TIME);
      } else {
        e->setReadTimeFrame(TimeFrame::CIRCLE);
      }
      break;
    case myrisa::dsp::LongPressButton::LONG_PRESS:
      e->setCircleToActiveLayer();
    }
  }
}

void Gko::processSelect() {
  float select_value  = params[SELECT_PARAM].getValue();
  float d_select = select_value - _last_select_value;
  float value_per_rotation = 2.4f;
  int n_increments_per_rotation = 16;
  float select_change_threshold = value_per_rotation / n_increments_per_rotation;
  if (select_change_threshold < fabs(d_select)) {
    for (int c = 0; c < channels(); c++) {
      myrisa::dsp::gko::Engine *e = _engines[c];

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

void Gko::processAlways(const ProcessArgs &args) {
  if (baseConnected()) {
    _from_signal = fromBase();
    _to_signal = toBase();

    outputs[PHASE_OUTPUT].setChannels(this->channels());
  }

  if (_button_divider.process()) {
    processButtons();
  }
}

void Gko::modulate() {
  processSelect();
}

void Gko::modulateChannel(int channel_i) {
  if (baseConnected()) {
    myrisa::dsp::gko::Engine *e = _engines[channel_i];
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

// TODO base off max of Gko & sig
int Gko::channels() {
  if (baseConnected() && _from_signal) {
    int input_channels = _from_signal->channels;
    if (_channels < input_channels) {
      return input_channels;
    }
  }

  return _channels;
}

void Gko::processChannel(const ProcessArgs& args, int channel_i) {
  if (!baseConnected()) {
    return;
  }

  myrisa::dsp::gko::Engine *e = _engines[channel_i];

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

void Gko::updateLights(const ProcessArgs &args) {
  // LIGHT + 0 = RED
  // LIGHT + 1 = GREEN
  // LIGHT + 2 = BLUE

  if (!baseConnected()) {
    return;
  }

  float signal_in_sum = 0.f;

  myrisa::dsp::gko::Engine *default_e = _engines[0];

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

  TimeFrame displayed_read_time_frame = default_e->_read_time_frame;
  RecordParams displayed_record_params = default_e->_record_params;
  float displayed_phase = default_e->_timeline_position.phase;

  float active_layer_signal_out_sum = 0.f;
  float sel_signal_out_sum = 0.f;

  bool record_active = false;
  for (int c = 0; c < channels(); c++) {
    signal_in_sum += _from_signal->signal[c];
    active_layer_signal_out_sum += _engines[c]->readActiveLayer();
    sel_signal_out_sum += _to_signal->sel_signal[c];
    if (record_active) {
      displayed_read_time_frame = _engines[c]->_read_time_frame;
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

  switch (displayed_record_params.mode) {
  case RecordParams::Mode::EXTEND:
    lights[RECORD_MODE_LIGHT + 0].value = 1.0;
    lights[RECORD_MODE_LIGHT + 1].value = 0.0;
    break;
  case RecordParams::Mode::DUB:
    lights[RECORD_MODE_LIGHT + 0].value = 0.0;
    lights[RECORD_MODE_LIGHT + 1].value = 1.0;
    break;
  }

  switch (displayed_record_params.time_frame) {
  case TimeFrame::TIME:
    lights[RECORD_TIME_FRAME_LIGHT + 0].value = 1.0;
    lights[RECORD_TIME_FRAME_LIGHT + 1].value = 0.0;
    break;
  case TimeFrame::CIRCLE:
    lights[RECORD_TIME_FRAME_LIGHT + 0].value = 0.0;
    lights[RECORD_TIME_FRAME_LIGHT + 1].value = 1.0;
    break;
  }

  switch (displayed_read_time_frame) {
  case TimeFrame::TIME:
    lights[READ_TIME_FRAME_LIGHT + 0].value = 1.0;
    lights[READ_TIME_FRAME_LIGHT + 1].value = 0.0;
    break;
  case TimeFrame::CIRCLE:
    lights[READ_TIME_FRAME_LIGHT + 0].value = 0.0;
    lights[READ_TIME_FRAME_LIGHT + 1].value = 1.0;
    break;
  }
}

void Gko::postProcessAlways(const ProcessArgs &args) {
  if (_light_divider.process()) {
    updateLights(args);
  }
}

void Gko::addChannel(int channel_i) {
  _engines[channel_i] = new myrisa::dsp::gko::Engine();
  _engines[channel_i]->_sample_time = _sampleTime;
}

void Gko::removeChannel(int channel_i) {
  delete _engines[channel_i];
  _engines[channel_i] = nullptr;
}

Model *modelGko = rack::createModel<Gko, GkoWidget>("Myrisa-Gko");
