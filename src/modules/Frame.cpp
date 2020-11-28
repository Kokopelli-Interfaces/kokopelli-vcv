#include "Frame.hpp"

Frame::Frame() {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  configParam(SELECT_PARAM, 0.f, 1.f, 0.f, "Select");
  configParam(SELECT_MODE_PARAM, 0.f, 1.f, 0.f, "Select Mode");
  configParam(SELECT_FUNCTION_PARAM, 0.f, 1.f, 0.f, "Select Function");
  configParam(TIME_FRAME_PARAM, 0.f, 1.f, 0.f, "Time Frame");
  configParam(DELTA_MODE_PARAM, 0.f, 1.f, 0.f, "Delta Mode");
  configParam(DELTA_CONTEXT_PARAM, 0.f, 1.f, 0.f, "Delta Context");
  configParam(DELTA_POWER_PARAM, 0.f, 1.f, 0.f, "Delta Power");

  setBaseModelPredicate([](Model *m) { return m == modelSignal; });
  _light_divider.setDivision(512);
  _button_divider.setDivision(4);

  _delta_mode_button.param = &params[DELTA_MODE_PARAM];
  _delta_context_button.param = &params[DELTA_CONTEXT_PARAM];
  _time_frame_button.param = &params[TIME_FRAME_PARAM];
}

void Frame::sampleRateChange() {
  _sampleTime = APP->engine->getSampleTime();
  for (int c = 0; c < channels(); c++) {
    _engines[c]->_sample_time = _sampleTime;
  }
}

void Frame::processButtons() {
  float sampleTime = _sampleTime * _button_divider.division;

  myrisa::dsp::LongPressButton::Event _delta_mode_event = _delta_mode_button.process(sampleTime);
  for (int c = 0; c < channels(); c++) {
    switch (_delta_mode_event) {
    case myrisa::dsp::LongPressButton::NO_PRESS:
      break;
    case myrisa::dsp::LongPressButton::SHORT_PRESS:
      if (_engines[c]->_delta.mode == Delta::Mode::DUB) {
        _engines[c]->_delta.mode = Delta::Mode::EXTEND;
      } else {
        _engines[c]->_delta.mode = Delta::Mode::DUB;
      }
      break;
    case myrisa::dsp::LongPressButton::LONG_PRESS:
      _engines[c]->_delta.mode = Delta::Mode::REPLACE;
      break;
    }
  }

  myrisa::dsp::LongPressButton::Event _delta_context_event = _delta_context_button.process(sampleTime);
  for (int c = 0; c < channels(); c++) {
    switch (_delta_context_event) {
    case myrisa::dsp::LongPressButton::NO_PRESS:
      break;
    case myrisa::dsp::LongPressButton::SHORT_PRESS:
      if (_engines[c]->_delta.context == Delta::Context::SCENE) {
        _engines[c]->_delta.context = Delta::Context::TIME;
      } else {
        _engines[c]->_delta.context = Delta::Context::SCENE;
      }
      break;
    case myrisa::dsp::LongPressButton::LONG_PRESS:
      _engines[c]->_delta.context = Delta::Context::LAYER;
      break;
    }
  }

  myrisa::dsp::LongPressButton::Event _time_frame_event = _time_frame_button.process(sampleTime);
  for (int c = 0; c < channels(); c++) {
    switch (_time_frame_event) {
    case myrisa::dsp::LongPressButton::NO_PRESS:
      break;
    case myrisa::dsp::LongPressButton::SHORT_PRESS:
      if (_engines[c]->_time_frame == TimeFrame::SECTION) {
        _engines[c]->_time_frame = TimeFrame::TIME;
      } else {
        _engines[c]->_time_frame = TimeFrame::SECTION;
      }
      break;
    case myrisa::dsp::LongPressButton::LONG_PRESS:
      _engines[c]->_time_frame = TimeFrame::LAYER;
      break;
    }
  }
}

void Frame::processAlways(const ProcessArgs &args) {
  if (baseConnected()) {
    _from_signal = fromBase();
    _to_signal = toBase();
  }

  if (_button_divider.process()) {
    processButtons();
  }
}

void Frame::modulateChannel(int channel_index) {
  myrisa::dsp::frame::Engine *e = _engines[channel_index];

  float scene_position = params[SELECT_PARAM].getValue() * (15);
  if (inputs[SCENE_INPUT].isConnected()) {
    scene_position += rack::clamp(inputs[SCENE_INPUT].getPolyVoltage(channel_index), -5.0f, 5.0f);
  }
  scene_position = rack::clamp(scene_position, 0.0f, 15.0f);
  e->updateScenePosition(scene_position);

  float delta_power = params[DELTA_POWER_PARAM].getValue();
  if (inputs[DELTA_POWER_INPUT].isConnected()) {
    float delta_power_port = inputs[DELTA_POWER_INPUT].getPolyVoltage(channel_index) / 10;
    delta_power = rack::clamp(delta_power_port + delta_power, 0.0f, 1.0f);
  }
  // taking to the power of 3 gives a more intuitive curve
  delta_power = rack::clamp(pow(delta_power, 3), 0.0f, 1.0f);
  e->updateDeltaPower(delta_power);

  e->_use_ext_phase = inputs[PHASE_INPUT].isConnected();
}

// TODO based off max
int Frame::channels() {
  if (baseConnected()) {
    int input_channels = _from_signal->channels;
    if (_channels < input_channels) {
      return input_channels;
    }
  }

  return _channels;
}

void Frame::processChannel(const ProcessArgs& args, int channel_index) {
  if (!baseConnected()) {
    return;
  }

  myrisa::dsp::frame::Engine *e = _engines[channel_index];

  if (e->_use_ext_phase) {
    e->_ext_phase = rack::clamp(
        inputs[PHASE_INPUT].getPolyVoltage(channel_index) / 10, 0.0f, 1.0f);
  }

  e->_in = _from_signal->signal[channel_index];
  e->step();
  _to_signal->signal[channel_index] = e->read();
}

void Frame::updateLights(const ProcessArgs &args) {
  Delta delta = _engines[0]->_delta;
  TimeFrame time_frame = _engines[0]->_time_frame;
  float phase = _engines[0]->_active_scene ? _engines[0]->_active_scene->_phase : 0.f;
  // display the engine with an active delta

  bool poly_delta = (inputs[DELTA_POWER_INPUT].isConnected() && 1 < inputs[DELTA_POWER_INPUT].getChannels());
  bool poly_phase = (inputs[PHASE_INPUT].isConnected() && 1 < inputs[PHASE_INPUT].getChannels());

  lights[DELTA_LIGHT + 1].value = !delta.active;

  if (poly_delta) {
    lights[DELTA_LIGHT + 0].value = 0.f;
    lights[DELTA_LIGHT + 2].setSmoothBrightness(
        delta.active - delta.power,
        _sampleTime * _light_divider.getDivision());
  } else {
    lights[DELTA_LIGHT + 0].setSmoothBrightness(
        delta.active - delta.power,
        _sampleTime * _light_divider.getDivision());
    lights[DELTA_LIGHT + 2].value = 0.f;
  }

  if (poly_phase) {
    lights[PHASE_LIGHT + 1].value = 0.f;
    lights[PHASE_LIGHT + 2].setSmoothBrightness(phase, _sampleTime * _light_divider.getDivision());
  } else {
    lights[PHASE_LIGHT + 1].setSmoothBrightness(
        phase, _sampleTime * _light_divider.getDivision());
    lights[PHASE_LIGHT + 2].value = 0.f;
  }

  switch (delta.mode) {
  case Delta::Mode::EXTEND:
    lights[DELTA_MODE_LIGHT + 0].value = 1.0;
    lights[DELTA_MODE_LIGHT + 1].value = 0.0;
    break;
  case Delta::Mode::DUB:
    lights[DELTA_MODE_LIGHT + 0].value = 1.0;
    lights[DELTA_MODE_LIGHT + 1].value = 1.0;
    break;
  case Delta::Mode::REPLACE:
    lights[DELTA_MODE_LIGHT + 0].value = 0.0;
    lights[DELTA_MODE_LIGHT + 1].value = 1.0;
    break;
  }

  switch (delta.context) {
  case Delta::Context::TIME:
    lights[DELTA_CONTEXT_LIGHT + 0].value = 1.0;
    lights[DELTA_CONTEXT_LIGHT + 1].value = 0.0;
    break;
  case Delta::Context::SCENE:
    lights[DELTA_CONTEXT_LIGHT + 0].value = 1.0;
    lights[DELTA_CONTEXT_LIGHT + 1].value = 1.0;
    break;
  case Delta::Context::LAYER:
    lights[DELTA_CONTEXT_LIGHT + 0].value = 0.0;
    lights[DELTA_CONTEXT_LIGHT + 1].value = 1.0;
    break;
  }

  switch (time_frame) {
  case TimeFrame::TIME:
    lights[TIME_FRAME_LIGHT + 0].value = 1.0;
    lights[TIME_FRAME_LIGHT + 1].value = 0.0;
    break;
  case TimeFrame::SECTION:
    lights[TIME_FRAME_LIGHT + 0].value = 1.0;
    lights[TIME_FRAME_LIGHT + 1].value = 1.0;
    break;
  case TimeFrame::LAYER:
    lights[TIME_FRAME_LIGHT + 0].value = 0.0;
    lights[TIME_FRAME_LIGHT + 1].value = 1.0;
    break;
  }
}

void Frame::postProcessAlways(const ProcessArgs &args) {
  if (_light_divider.process()) {
    updateLights(args);
  }
}

void Frame::addChannel(int channel_index) {
  _engines[channel_index] = new myrisa::dsp::frame::Engine();
  _engines[channel_index]->_sample_time = _sampleTime;
}

void Frame::removeChannel(int channel_index) {
  delete _engines[channel_index];
  _engines[channel_index] = nullptr;
}


struct FrameValueDisplay : TextBox {
	Frame *_module;
	int _previous_displayed_value = 0;

	FrameValueDisplay(Frame *m) : TextBox() {
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

	void updateDisplayValue(int v) {
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
      updateDisplayValue(327);
		}
	}

};

struct FrameWidget : ModuleWidget {
  const int hp = 4;
  FrameValueDisplay *current_selection;
  FrameValueDisplay *total_selections;
  FrameValueDisplay *current_section;
  FrameValueDisplay *total_sections;
  FrameValueDisplay *current_section_division;
  FrameValueDisplay *total_section_divisions;
  FrameValueDisplay *time;

  FrameWidget(Frame *module) {
    setModule(module);
    box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Frame.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<Rogan1HPSWhite>(mm2px(Vec(5.333, 21.157)), module, Frame::SELECT_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(17.774, 33.464)), module, Frame::SELECT_MODE_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(1.618, 33.463)), module, Frame::SELECT_FUNCTION_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(9.64, 50.315)), module, Frame::TIME_FRAME_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(17.849, 66.389)), module, Frame::DELTA_CONTEXT_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(1.447, 66.412)), module, Frame::DELTA_MODE_PARAM));
		addParam(createParam<Rogan3PDarkRed>(mm2px(Vec(5.334, 73.21)), module, Frame::DELTA_POWER_PARAM));

		addInput(createInput<PJ301MPort>(mm2px(Vec(8.522, 37.132)), module, Frame::SCENE_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(8.384, 88.878)), module, Frame::DELTA_POWER_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(1.798, 108.114)), module, Frame::PHASE_INPUT));

		addOutput(createOutput<PJ301MPort>(mm2px(Vec(15.306, 108.114)), module, Frame::PHASE_OUTPUT));

		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(3.083, 34.928)), module, Frame::SELECT_FUNCTION_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(19.239, 34.928)), module, Frame::SELECT_MODE_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(11.099, 51.744)), module, Frame::TIME_FRAME_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(11.097, 61.939)), module, Frame::DELTA_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(19.313, 67.854)), module, Frame::DELTA_CONTEXT_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(2.911, 67.877)), module, Frame::DELTA_MODE_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(11.181, 110.546)), module, Frame::PHASE_LIGHT));

    auto display_size = mm2px(Vec(9.096, 4.327));

		current_selection = new FrameValueDisplay(module);
    current_selection->box.pos = mm2px(Vec(2.921, 16.235));
    current_selection->box.size = display_size;
    addChild(current_selection);

    total_selections = new FrameValueDisplay(module);
    total_selections->box.pos = mm2px(Vec(13.387, 16.236));
    total_selections->box.size = display_size;
    addChild(total_selections);

    display_size = mm2px(Vec(6.837, 4.327));

    current_section = new FrameValueDisplay(module);
    current_section->box.pos = mm2px(Vec(1.071, 48.611));
    current_section->box.size = display_size;
    addChild(current_section);

    total_sections = new FrameValueDisplay(module);
    total_sections->box.pos = mm2px(Vec(1.071, 53.716));
    total_sections->box.size = display_size;
    addChild(total_sections);

    current_section_division = new FrameValueDisplay(module);
    current_section_division->box.pos = mm2px(Vec(17.511, 48.611));
    current_section_division->box.size = display_size;
    addChild(current_section_division);

    total_section_divisions = new FrameValueDisplay(module);
    total_section_divisions->box.pos = mm2px(Vec(17.511, 53.751));
    total_section_divisions->box.size = display_size;
    addChild(total_section_divisions);

    display_size = mm2px(Vec(21.44, 4.327));

    time = new FrameValueDisplay(module);
    time->box.pos = mm2px(Vec(1.974, 99.568));
    time->box.size = display_size;
    addChild(time);
}
};

Model *modelFrame = rack::createModel<Frame, FrameWidget>("Myrisa-Frame");
