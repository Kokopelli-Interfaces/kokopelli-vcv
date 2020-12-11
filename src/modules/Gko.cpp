#include "Gko.hpp"
#include "GkoWidget.hpp"

Gko::Gko() {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  configParam(SELECT_PARAM, 0.f, 1.f, 0.f, "Select");
  configParam(SELECT_MODE_PARAM, 0.f, 1.f, 0.f, "Select Mode");
  configParam(SELECT_FUNCTION_PARAM, 0.f, 1.f, 0.f, "Select Function");
  configParam(READ_TIME_FRAME_PARAM, 0.f, 1.f, 0.f, "Read Time Frame");
  configParam(RECORD_MODE_PARAM, 0.f, 1.f, 0.f, "Record Mode");
  configParam(RECORD_TIME_FRAME_PARAM, 0.f, 1.f, 0.f, "Record Time Frame");
  configParam(RECORD_PARAM, 0.f, 1.f, 0.f, "Record Strength");

  setBaseModelPredicate([](Model *m) { return m == modelSignal; });
  _light_divider.setDivision(512);
  _button_divider.setDivision(4);

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

  myrisa::dsp::LongPressButton::Event _record_mode_event = _record_mode_button.process(sampleTime);
  for (int c = 0; c < channels(); c++) {
    switch (_record_mode_event) {
    case myrisa::dsp::LongPressButton::NO_PRESS:
      break;
    case myrisa::dsp::LongPressButton::SHORT_PRESS:
      if (_engines[c]->_record_params.mode == RecordParams::Mode::DUB) {
        _engines[c]->_record_params.mode = RecordParams::Mode::EXTEND;
      } else {
        _engines[c]->_record_params.mode = RecordParams::Mode::DUB;
      }
      break;
    case myrisa::dsp::LongPressButton::LONG_PRESS:
      _engines[c]->_record_params.mode = RecordParams::Mode::REPLACE;
      break;
    }
  }

  myrisa::dsp::LongPressButton::Event _record_time_frame_event = _record_time_frame_button.process(sampleTime);
  for (int c = 0; c < channels(); c++) {
    switch (_record_time_frame_event) {
    case myrisa::dsp::LongPressButton::NO_PRESS:
      break;
    case myrisa::dsp::LongPressButton::SHORT_PRESS:
      if (_engines[c]->_record_params.time_frame == TimeFrame::SELECTED_LAYERS) {
        _engines[c]->_record_params.time_frame = TimeFrame::TIMELINE;
      } else {
        _engines[c]->_record_params.time_frame = TimeFrame::SELECTED_LAYERS;
      }
      break;
    case myrisa::dsp::LongPressButton::LONG_PRESS:
        _engines[c]->_record_params.time_frame = TimeFrame::ACTIVE_LAYER;
      break;
    }
  }

  myrisa::dsp::LongPressButton::Event _read_time_frame_event = _read_time_frame_button.process(sampleTime);
  for (int c = 0; c < channels(); c++) {
    switch (_read_time_frame_event) {
    case myrisa::dsp::LongPressButton::NO_PRESS:
      break;
    case myrisa::dsp::LongPressButton::SHORT_PRESS:
      if (_engines[c]->_read_time_frame == TimeFrame::SELECTED_LAYERS) {
        _engines[c]->_read_time_frame = TimeFrame::TIMELINE;
      } else {
        _engines[c]->_read_time_frame = TimeFrame::SELECTED_LAYERS;
      }
      break;
    case myrisa::dsp::LongPressButton::LONG_PRESS:
      _engines[c]->_read_time_frame = TimeFrame::ACTIVE_LAYER;
      break;
    }
  }
}

void Gko::processAlways(const ProcessArgs &args) {
  if (baseConnected()) {
    _from_signal = fromBase();
    _to_signal = toBase();
  }

  if (_button_divider.process()) {
    processButtons();
  }
}

void Gko::modulateChannel(int channel_i) {
  if (baseConnected()) {
    myrisa::dsp::gko::Engine *e = _engines[channel_i];
    float record_strength = params[RECORD_PARAM].getValue();
    if (inputs[RECORD_INPUT].isConnected()) {
      record_strength *= rack::clamp(inputs[RECORD_INPUT].getPolyVoltage(channel_i) / 10.f, 0.f, 1.0f);
    }
    // taking to the strength of 3 gives a more intuitive curve
    record_strength = pow(record_strength, 3);
    e->_record_params.strength = record_strength;

    e->_use_ext_phase = inputs[PHASE_INPUT].isConnected();

    e->_active_layer_i = 0;

    e->_options = _options;

    // TODO have knob, now it just selects all layers
    std::vector<unsigned int> selected_layers_idx;
    selected_layers_idx.resize(e->_timeline.layers.size());
    std::iota(std::begin(selected_layers_idx), std::end(selected_layers_idx), 0);
    e->_selected_layers_idx = selected_layers_idx;

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
    e->_ext_phase = rack::clamp(inputs[PHASE_INPUT].getPolyVoltage(channel_i) / 10, 0.f, 1.0f);
  }

  if (outputs[PHASE_OUTPUT].isConnected()) {
    outputs[PHASE_OUTPUT].setVoltage(e->_timeline_position.phase * 10, channel_i);
  }

  e->_record_params.in = _from_signal->signal[channel_i];
  e->step();
  _to_signal->signal[channel_i] = e->read();
}

void Gko::updateLights(const ProcessArgs &args) {
  // LIGHT + 0 = RED
  // LIGHT + 1 = GREEN
  // LIGHT + 2 = BLUE

  if (!baseConnected()) {
    return;
  }

  float signal_in_sum = 0.f;
  float signal_out_sum = 0.f;

  bool poly_record = (inputs[RECORD_INPUT].isConnected() && 1 < inputs[RECORD_INPUT].getChannels());

  TimeFrame displayed_read_time_frame = _engines[0]->_read_time_frame;
  RecordParams displayed_record_params = _engines[0]->_record_params;
  float displayed_phase = _engines[0]->_timeline_position.phase;

  bool record_active = false;
  for (int c = 0; c < channels(); c++) {
    signal_in_sum += _from_signal->signal[c];
    signal_out_sum += _to_signal->signal[c];
    if (record_active) {
      displayed_read_time_frame = _engines[c]->_read_time_frame;
      displayed_record_params = _engines[c]->_record_params;
      displayed_phase = _engines[c]->_timeline_position.phase;
    }
  }

  signal_in_sum = rack::clamp(signal_in_sum, 0.f, 1.f);
  signal_out_sum = rack::clamp(signal_out_sum, 0.f, 1.f);

  // TODO make me show the layer output that is selected, not all
  lights[RECORD_LIGHT + 1].setSmoothBrightness(signal_out_sum, _sampleTime * _light_divider.getDivision());

  if (displayed_record_params.active()) {
    int light_colour = poly_record ? 2 : 0;
    lights[RECORD_LIGHT + light_colour].setSmoothBrightness(signal_in_sum, _sampleTime * _light_divider.getDivision());
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

  switch (displayed_record_params.mode) {
  case RecordParams::Mode::EXTEND:
    lights[RECORD_MODE_LIGHT + 0].value = 1.0;
    lights[RECORD_MODE_LIGHT + 1].value = 0.0;
    break;
  case RecordParams::Mode::DUB:
    lights[RECORD_MODE_LIGHT + 0].value = 1.0;
    lights[RECORD_MODE_LIGHT + 1].value = 1.0;
    break;
  case RecordParams::Mode::REPLACE:
    lights[RECORD_MODE_LIGHT + 0].value = 0.0;
    lights[RECORD_MODE_LIGHT + 1].value = 1.0;
    break;
  }

  switch (displayed_record_params.time_frame) {
  case TimeFrame::TIMELINE:
    lights[RECORD_TIME_FRAME_LIGHT + 0].value = 1.0;
    lights[RECORD_TIME_FRAME_LIGHT + 1].value = 0.0;
    break;
  case TimeFrame::SELECTED_LAYERS:
    lights[RECORD_TIME_FRAME_LIGHT + 0].value = 1.0;
    lights[RECORD_TIME_FRAME_LIGHT + 1].value = 1.0;
    break;
  case TimeFrame::ACTIVE_LAYER:
    lights[RECORD_TIME_FRAME_LIGHT + 0].value = 0.0;
    lights[RECORD_TIME_FRAME_LIGHT + 1].value = 1.0;
    break;
  }

  switch (displayed_read_time_frame) {
  case TimeFrame::TIMELINE:
    lights[READ_TIME_FRAME_LIGHT + 0].value = 1.0;
    lights[READ_TIME_FRAME_LIGHT + 1].value = 0.0;
    break;
  case TimeFrame::SELECTED_LAYERS:
    lights[READ_TIME_FRAME_LIGHT + 0].value = 1.0;
    lights[READ_TIME_FRAME_LIGHT + 1].value = 1.0;
    break;
  case TimeFrame::ACTIVE_LAYER:
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
