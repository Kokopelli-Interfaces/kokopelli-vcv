#include "Gko.hpp"
#include "GkoWidget.hpp"

using namespace myrisa;

Gko::Gko() {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  configParam(SELECT_PARAM, -INFINITY, INFINITY, 0.f, "Select");
  configParam(SELECT_MODE_PARAM, 0.f, 1.f, 0.f, "Select Mode");
  configParam(SELECT_FUNCTION_PARAM, 0.f, 1.f, 0.f, "Select Function");
  configParam(READ_TIME_FRAME_PARAM, 0.f, 1.f, 0.f, "Read Time Frame");
  configParam(RECORD_MODE_PARAM, 0.f, 1.f, 0.f, "Record Mode");
  configParam(RECORD_TIME_FRAME_PARAM, 0.f, 1.f, 0.f, "Record Time Frame");
  configParam(RECORD_PARAM, 0.f, 1.f, 0.f, "Record Strength");

  _light_divider.setDivision(512);
  _button_divider.setDivision(4);

  _select_function_button.param = &params[SELECT_FUNCTION_PARAM];
  _select_mode_button.param = &params[SELECT_MODE_PARAM];

  _record_mode_button.param = &params[RECORD_MODE_PARAM];
  _record_time_frame_button.param = &params[RECORD_TIME_FRAME_PARAM];
  _read_time_frame_button.param = &params[READ_TIME_FRAME_PARAM];

  printf("gko channels GKO: %ld\n", gko_connections.size());
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
      if (e->isSelected(e->_active_layer_i) && !e->_new_layer_active) {
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

    switch (_record_mode_event) {
    case myrisa::dsp::LongPressButton::NO_PRESS:
      break;
    case myrisa::dsp::LongPressButton::SHORT_PRESS:
      if (e->_record_interface.mode == RecordInterface::Mode::DUB) {
        e->_record_interface.mode = RecordInterface::Mode::EXTEND;
      } else {
        e->_record_interface.mode = RecordInterface::Mode::DUB;
      }
      break;
    case myrisa::dsp::LongPressButton::LONG_PRESS:
      e->_record_interface.mode = RecordInterface::Mode::REPLACE;
      break;
    }

    switch (_record_time_frame_event) {
    case myrisa::dsp::LongPressButton::NO_PRESS:
      break;
    case myrisa::dsp::LongPressButton::SHORT_PRESS:
      if (e->_record_interface.time_frame == TimeFrame::SELECTED_LAYERS) {
        e->_record_interface.time_frame = TimeFrame::TIMELINE;
      } else {
        e->_record_interface.time_frame = TimeFrame::SELECTED_LAYERS;
      }
      break;
    case myrisa::dsp::LongPressButton::LONG_PRESS:
        e->_record_interface.time_frame = TimeFrame::ACTIVE_LAYER;
      break;
    }

    switch (_read_time_frame_event) {
    case myrisa::dsp::LongPressButton::NO_PRESS:
      break;
    case myrisa::dsp::LongPressButton::SHORT_PRESS:
      if (e->_read_time_frame == TimeFrame::SELECTED_LAYERS) {
        e->_read_time_frame = TimeFrame::TIMELINE;
      } else {
        e->_read_time_frame = TimeFrame::SELECTED_LAYERS;
      }
      break;
    case myrisa::dsp::LongPressButton::LONG_PRESS:
      e->_read_time_frame = TimeFrame::ACTIVE_LAYER;
      break;
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

      int new_active_layer_i = e->_active_layer_i;
      if (0.f < d_select) {
        if (e->_active_layer_i < e->_timeline.layers.size()) {
          new_active_layer_i++;
        }
      } else if (0 < e->_active_layer_i) {
        new_active_layer_i--;
      }

      if (new_active_layer_i == (int)e->_timeline.layers.size()) {
        e->_new_layer_active = true;
      } else {
        e->_new_layer_active = false;
        e->_active_layer_i = new_active_layer_i;
      }
    }

    _last_select_value = select_value;
  }
}

void Gko::processAlways(const ProcessArgs &args) {
  if (_button_divider.process()) {
    processButtons();
  }
}

void Gko::modulate() {
  processSelect();
}

bool Gko::hasConnections() {
  return 0 < gko_connections.size();
}

void Gko::modulateChannel(int channel_i) {
  if (hasConnections()) {
    myrisa::dsp::gko::Engine *e = _engines[channel_i];
    float record_strength = params[RECORD_PARAM].getValue();
    if (inputs[RECORD_INPUT].isConnected()) {
      record_strength *= rack::clamp(inputs[RECORD_INPUT].getPolyVoltage(channel_i) / 10.f, 0.f, 1.0f);
    }

    // taking to the strength of 2 gives a more intuitive curve
    record_strength = pow(record_strength, 2);
    e->_record_interface.strength = record_strength;

    e->_use_ext_phase = inputs[PHASE_INPUT].isConnected();

    e->_options = _options;

    // FIXME
    e->_signal_type = gko_connections[0]->signal_type;
  }
}

// TODO base off max of Gko & sig
int Gko::channels() {
  if (hasConnections()) {
    int input_channels = gko_connections[0]->send_channels;
    if (_channels < input_channels) {
      return input_channels;
    }
  }

  return _channels;
}

void Gko::processChannel(const ProcessArgs& args, int channel_i) {
  if (!hasConnections()) {
    return;
  }

  myrisa::dsp::gko::Engine *e = _engines[channel_i];

  if (inputs[PHASE_INPUT].isConnected()) {
    e->_ext_phase = rack::clamp(inputs[PHASE_INPUT].getPolyVoltage(channel_i) / 10, 0.f, 1.0f);
  }

  if (outputs[PHASE_OUTPUT].isConnected()) {
    outputs[PHASE_OUTPUT].setVoltage(e->_timeline_position.phase * 10, channel_i);
  }

  // FIXME array
  e->_record_interface.in = gko_connections[0]->to[channel_i];
  e->step();
  gko_connections[0]->from[channel_i] = e->read();
}

void Gko::updateLights(const ProcessArgs &args) {
  // LIGHT + 0 = RED
  // LIGHT + 1 = GREEN
  // LIGHT + 2 = BLUE

  if (!hasConnections()) {
    return;
  }

  myrisa::dsp::gko::Engine *default_e = _engines[0];

  lights[SELECT_FUNCTION_LIGHT + 0].value = 0.f;
  lights[SELECT_FUNCTION_LIGHT + 1].value = 0.f;
  if (default_e->_new_layer_active || default_e->_recording_layer != nullptr) {
    if (default_e->_select_new_layers) {
      lights[SELECT_FUNCTION_LIGHT + 1].value = 1.f;
    } else {
      lights[SELECT_FUNCTION_LIGHT + 1].value = 0.f;
    }
  } else if (default_e->isSelected(default_e->_active_layer_i)) {
    lights[SELECT_FUNCTION_LIGHT + 1].value = 1.f;
    if (default_e->_selected_layers_idx.size() == 1) {
      lights[SELECT_FUNCTION_LIGHT + 0].value = 1.f;
    }
  }

  bool poly_record = (inputs[RECORD_INPUT].isConnected() && 1 < inputs[RECORD_INPUT].getChannels());

  TimeFrame displayed_read_time_frame = default_e->_read_time_frame;
  RecordInterface displayed_record_interface = default_e->_record_interface;
  float displayed_phase = default_e->_timeline_position.phase;

  float in_sum = 0.f;
  float out_sum = 0.f;

  bool record_active = false;
  for (int c = 0; c < channels(); c++) {
    in_sum += gko_connections[0]->to[c];
    out_sum += gko_connections[0]->from[c];
    if (record_active) {
      displayed_read_time_frame = _engines[c]->_read_time_frame;
      displayed_record_interface = _engines[c]->_record_interface;
      displayed_phase = _engines[c]->_timeline_position.phase;
    }
  }

  in_sum = rack::clamp(in_sum, 0.f, 1.f);
  out_sum = rack::clamp(out_sum, 0.f, 1.f);

  // TODO make me show the layer output that is selected, not all
  lights[RECORD_LIGHT + 1].setSmoothBrightness(out_sum, _sampleTime * _light_divider.getDivision());

  if (displayed_record_interface.active()) {
    int light_colour = poly_record ? 2 : 0;
    lights[RECORD_LIGHT + light_colour].setSmoothBrightness(in_sum, _sampleTime * _light_divider.getDivision());
  } else {
    lights[RECORD_LIGHT + 0].value = 0.f;
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

  switch (displayed_record_interface.mode) {
  case RecordInterface::Mode::EXTEND:
    lights[RECORD_MODE_LIGHT + 0].value = 1.0;
    lights[RECORD_MODE_LIGHT + 1].value = 0.0;
    break;
  case RecordInterface::Mode::DUB:
    lights[RECORD_MODE_LIGHT + 0].value = 0.0;
    lights[RECORD_MODE_LIGHT + 1].value = 1.0;
    break;
  case RecordInterface::Mode::REPLACE:
    lights[RECORD_MODE_LIGHT + 0].value = 1.0;
    lights[RECORD_MODE_LIGHT + 1].value = 1.0;
    break;
  }

  switch (displayed_record_interface.time_frame) {
  case TimeFrame::SELECTED_LAYERS:
    lights[RECORD_TIME_FRAME_LIGHT + 0].value = 1.0;
    lights[RECORD_TIME_FRAME_LIGHT + 1].value = 0.0;
    break;
  case TimeFrame::TIMELINE:
    lights[RECORD_TIME_FRAME_LIGHT + 0].value = 0.0;
    lights[RECORD_TIME_FRAME_LIGHT + 1].value = 1.0;
    break;
  case TimeFrame::ACTIVE_LAYER:
    lights[RECORD_TIME_FRAME_LIGHT + 0].value = 1.0;
    lights[RECORD_TIME_FRAME_LIGHT + 1].value = 1.0;
    break;
  }

  switch (displayed_read_time_frame) {
  case TimeFrame::TIMELINE:
    lights[READ_TIME_FRAME_LIGHT + 0].value = 1.0;
    lights[READ_TIME_FRAME_LIGHT + 1].value = 0.0;
    break;
  case TimeFrame::SELECTED_LAYERS:
    lights[READ_TIME_FRAME_LIGHT + 0].value = 0.0;
    lights[READ_TIME_FRAME_LIGHT + 1].value = 1.0;
    break;
  case TimeFrame::ACTIVE_LAYER:
    lights[READ_TIME_FRAME_LIGHT + 0].value = 1.0;
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
