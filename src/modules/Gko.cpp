#include "Gko.hpp"

Gko::Gko() {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  configParam(SELECT_PARAM, 0.f, 1.f, 0.f, "Select");
  configParam(SELECT_MODE_PARAM, 0.f, 1.f, 0.f, "Select Mode");
  configParam(SELECT_FUNCTION_PARAM, 0.f, 1.f, 0.f, "Select Function");
  configParam(TIME_FRAME_PARAM, 0.f, 1.f, 0.f, "Time Frame");
  configParam(RECORD_MODE_PARAM, 0.f, 1.f, 0.f, "Record Mode");
  configParam(RECORD_TIME_FRAME_PARAM, 0.f, 1.f, 0.f, "Record Time Frame");
  configParam(RECORD_PARAM, 0.f, 1.f, 0.f, "Record Strength");

  setBaseModelPredicate([](Model *m) { return m == modelSignal; });
  _light_divider.setDivision(512);
  _button_divider.setDivision(4);

  _record_mode_button.param = &params[RECORD_MODE_PARAM];
  _record_time_frame_button.param = &params[RECORD_TIME_FRAME_PARAM];
  _time_frame_button.param = &params[TIME_FRAME_PARAM];
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
      if (_engines[c]->_record.mode == RecordParams::Mode::DUB) {
        _engines[c]->setRecordMode(RecordParams::Mode::EXTEND);
      } else {
        _engines[c]->setRecordMode(RecordParams::Mode::DUB);
      }
      break;
    case myrisa::dsp::LongPressButton::LONG_PRESS:
      _engines[c]->setRecordMode(RecordParams::Mode::REPLACE);
      break;
    }
  }

  myrisa::dsp::LongPressButton::Event _record_time_frame_event = _record_time_frame_button.process(sampleTime);
  for (int c = 0; c < channels(); c++) {
    switch (_record_time_frame_event) {
    case myrisa::dsp::LongPressButton::NO_PRESS:
      break;
    case myrisa::dsp::LongPressButton::SHORT_PRESS:
      if (_engines[c]->_record.time_frame == TimeFrame::SELECTED_LAYERS) {
        _engines[c]->setRecordTimeFrame(TimeFrame::TIMELINE);
      } else {
        _engines[c]->setRecordTimeFrame(TimeFrame::SELECTED_LAYERS);
      }
      break;
    case myrisa::dsp::LongPressButton::LONG_PRESS:
        _engines[c]->setRecordTimeFrame(TimeFrame::ACTIVE_LAYER);
      break;
    }
  }

  myrisa::dsp::LongPressButton::Event _time_frame_event = _time_frame_button.process(sampleTime);
  for (int c = 0; c < channels(); c++) {
    switch (_time_frame_event) {
    case myrisa::dsp::LongPressButton::NO_PRESS:
      break;
    case myrisa::dsp::LongPressButton::SHORT_PRESS:
      if (_engines[c]->_time_frame == TimeFrame::SELECTED_LAYERS) {
        _engines[c]->_time_frame = TimeFrame::TIMELINE;
      } else {
        _engines[c]->_time_frame = TimeFrame::SELECTED_LAYERS;
      }
      break;
    case myrisa::dsp::LongPressButton::LONG_PRESS:
      _engines[c]->_time_frame = TimeFrame::ACTIVE_LAYER;
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

void Gko::modulateChannel(int channel_index) {
  if (baseConnected()) {
    myrisa::dsp::gko::Engine *e = _engines[channel_index];
    float record_strength = params[RECORD_PARAM].getValue();
    if (inputs[RECORD_INPUT].isConnected()) {
      float record_strength_port = inputs[RECORD_INPUT].getPolyVoltage(channel_index) / 10;
      record_strength = rack::clamp(record_strength_port + record_strength, 0.f, 1.0f);
    }
    // taking to the strength of 3 gives a more intuitive curve
    record_strength = rack::clamp(pow(record_strength, 3), 0.f, 1.0f);
    e->setRecordStrength(record_strength);

    e->_use_ext_phase = inputs[PHASE_INPUT].isConnected();

    // TODO have knob, now it just selects all layers
    std::vector<int> selected_layers_idx;
    selected_layers_idx.resize(e->_timeline.layers.size());
    std::iota(std::begin(selected_layers_idx), std::end(selected_layers_idx), 0);
    e->_selected_layers = selected_layers_idx;
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

void Gko::processChannel(const ProcessArgs& args, int channel_index) {
  if (!baseConnected()) {
    return;
  }

  myrisa::dsp::gko::Engine *e = _engines[channel_index];

  if (inputs[PHASE_INPUT].isConnected()) {
    e->_ext_phase = rack::clamp(inputs[PHASE_INPUT].getPolyVoltage(channel_index) / 10, 0.f, 1.0f);
  }

  if (outputs[PHASE_OUTPUT].isConnected()) {
    float phase = rack::math::eucMod(e->_time, 1.0f);
    outputs[PHASE_OUTPUT].setVoltage(phase * 10, channel_index);
  }

  e->_record.in = _from_signal->signal[channel_index];
  e->step();
  _to_signal->signal[channel_index] = e->read();
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

  TimeFrame displayed_time_frame = _engines[0]->_time_frame;
  RecordParams displayed_record = _engines[0]->_record;
  float displayed_phase = rack::math::eucMod(_engines[0]->_time, 1.0f);

  bool record_active = false;
  for (int c = 0; c < channels(); c++) {
    signal_in_sum += _from_signal->signal[c];
    signal_out_sum += _to_signal->signal[c];
    record_active = !record_active ? _engines[c]->_record.active : record_active;
    if (record_active) {
      displayed_time_frame = _engines[c]->_time_frame;
      displayed_record = _engines[c]->_record;
      displayed_phase = rack::math::eucMod(_engines[c]->_time, 1.0f);
    }
  }

  signal_in_sum = rack::clamp(signal_in_sum, 0.f, 1.f);
  signal_out_sum = rack::clamp(signal_out_sum, 0.f, 1.f);

  // TODO make me show the layer output that is selected, not all
  lights[RECORD_LIGHT + 1].setSmoothBrightness(signal_out_sum, _sampleTime * _light_divider.getDivision());

  if (record_active) {
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

  switch (displayed_record.mode) {
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

  switch (displayed_record.time_frame) {
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

  switch (displayed_time_frame) {
  case TimeFrame::TIMELINE:
    lights[TIME_FRAME_LIGHT + 0].value = 1.0;
    lights[TIME_FRAME_LIGHT + 1].value = 0.0;
    break;
  case TimeFrame::SELECTED_LAYERS:
    lights[TIME_FRAME_LIGHT + 0].value = 1.0;
    lights[TIME_FRAME_LIGHT + 1].value = 1.0;
    break;
  case TimeFrame::ACTIVE_LAYER:
    lights[TIME_FRAME_LIGHT + 0].value = 0.0;
    lights[TIME_FRAME_LIGHT + 1].value = 1.0;
    break;
  }
}

void Gko::postProcessAlways(const ProcessArgs &args) {
  if (_light_divider.process()) {
    updateLights(args);
  }
}

void Gko::addChannel(int channel_index) {
  _engines[channel_index] = new myrisa::dsp::gko::Engine();
  _engines[channel_index]->_sample_time = _sampleTime;
}

void Gko::removeChannel(int channel_index) {
  delete _engines[channel_index];
  _engines[channel_index] = nullptr;
}


struct GkoValueDisplay : TextBox {
	Gko *_module;
	int _previous_displayed_value = 0;

	GkoValueDisplay(Gko *m) : TextBox() {
      _module = m;
      // font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/nunito/Nunito-Bold.ttf"));
    // font = APP->window->loadFont(asset::plugin(pluginInstance, "/res/fonts/Overpass-Regular.ttf"));
    font = APP->window->loadFont(asset::plugin(pluginInstance, "/res/fonts/Overpass-Bold.ttf"));
    font_size = 12;
    letter_spacing = 0.f;
    // backgroundColor = nvgRGB(0xee, 0xe8, 0xd5); // solarized base2
    // backgroundColor = nvgRGB(0x93, 0xa1, 0xa1); // solarized base1
    // backgroundColor = nvgRGB(0x78, 0x78, 0x78); // blendish default
    backgroundColor = nvgRGB(0xd7, 0xda, 0xec); // blend
    // backgroundColor = nvgRGB(0xff, 0xff, 0xff); // custom
    box.size = mm2px(Vec(6.902, 3.283));
    textOffset = Vec(box.size.x * 0.5f, 0.f);
  }

	void setDisplayValue(int v) {
		std::string s;
		if(v != _previous_displayed_value) {
			_previous_displayed_value = v;
      s = string::f("%d", v);
      setText(s);
		}
	}

	void step() override {
		TextBox::step();
		if(_module) {
      setDisplayValue(327);
		}
	}

};

struct GkoWidget : ModuleWidget {
  const int hp = 4;
  GkoValueDisplay *current_selection;
  GkoValueDisplay *total_selections;
  GkoValueDisplay *current_section;
  GkoValueDisplay *total_sections;
  GkoValueDisplay *current_section_division;
  GkoValueDisplay *total_section_divisions;
  GkoValueDisplay *time;

  GkoWidget(Gko *module) {
    setModule(module);
    box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Gko.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<Rogan1HPSWhite>(mm2px(Vec(5.333, 21.157)), module, Gko::SELECT_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(17.774, 33.464)), module, Gko::SELECT_MODE_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(1.618, 33.463)), module, Gko::SELECT_FUNCTION_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(9.64, 51.330)), module, Gko::TIME_FRAME_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(1.447, 65.437)), module, Gko::RECORD_MODE_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(17.849, 65.436)), module, Gko::RECORD_TIME_FRAME_PARAM));
		addParam(createParam<Rogan3PDarkRed>(mm2px(Vec(5.334, 73.118)), module, Gko::RECORD_PARAM));

		addInput(createInput<PJ301MPort>(mm2px(Vec(8.522, 37.132)), module, Gko::SCENE_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(8.384, 88.869)), module, Gko::RECORD_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(1.798, 108.114)), module, Gko::PHASE_INPUT));

		addOutput(createOutput<PJ301MPort>(mm2px(Vec(15.306, 108.114)), module, Gko::PHASE_OUTPUT));

		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(3.083, 34.928)), module, Gko::SELECT_FUNCTION_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(19.239, 34.928)), module, Gko::SELECT_MODE_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(11.155, 52.736)), module, Gko::TIME_FRAME_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(11.097, 62.77)), module, Gko::RECORD_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(2.912, 66.901)), module, Gko::RECORD_MODE_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(19.313, 66.901)), module, Gko::RECORD_TIME_FRAME_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(11.181, 110.546)), module, Gko::PHASE_LIGHT));

    auto display_size = mm2px(Vec(9.096, 4.327));

		current_selection = new GkoValueDisplay(module);
    current_selection->box.pos = mm2px(Vec(2.921, 16.235));
    current_selection->box.size = display_size;
    addChild(current_selection);

    total_selections = new GkoValueDisplay(module);
    total_selections->box.pos = mm2px(Vec(13.387, 16.236));
    total_selections->box.size = display_size;
    addChild(total_selections);

    display_size = mm2px(Vec(6.837, 4.327));

    current_section = new GkoValueDisplay(module);
    current_section->box.pos = mm2px(Vec(1.071, 48.917));
    current_section->box.size = display_size;
    addChild(current_section);

    current_section_division = new GkoValueDisplay(module);
    current_section_division->box.pos = mm2px(Vec(17.511, 48.917));
    current_section_division->box.size = display_size;
    addChild(current_section_division);

    total_sections = new GkoValueDisplay(module);
    total_sections->box.pos = mm2px(Vec(1.071, 54.022));
    total_sections->box.size = display_size;
    addChild(total_sections);

    total_section_divisions = new GkoValueDisplay(module);
    total_section_divisions->box.pos = mm2px(Vec(17.511, 54.022));
    total_section_divisions->box.size = display_size;
    addChild(total_section_divisions);

    display_size = mm2px(Vec(21.44, 4.327));

    time = new GkoValueDisplay(module);
    time->box.pos = mm2px(Vec(1.974, 99.568));
    time->box.size = display_size;
    addChild(time);
}
};

Model *modelGko = rack::createModel<Gko, GkoWidget>("Myrisa-Gko");
