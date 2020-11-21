#include "Frame.hpp"

Frame::Frame() {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  configParam(SECTION_PARAM, 0.f, 1.f, 0.f, "Section");
  configParam(PLAY_PARAM, 0.f, 1.f, 0.f, "Play Layer");
  configParam(NEXT_PARAM, 0.f, 1.f, 0.f, "Next Layer");
  configParam(PREV_PARAM, 0.f, 1.f, 0.f, "Prev Layer");
  configParam(UNDO_PARAM, 0.f, 1.f, 0.f, "Undo");
  configParam(RECORD_MODE_PARAM, 0.f, 1.f, 0.f, "Record Mode");
  configParam(DELTA_PARAM, 0.f, 1.f, 0.5f, "Delta");
  setBaseModelPredicate([](Model *m) { return m == modelSignal; });
  light_divider.setDivision(16);
}

void Frame::sampleRateChange() {
  _sampleTime = APP->engine->getSampleTime();
}

void Frame::processAlways(const ProcessArgs &args) {
  if (baseConnected()) {
    _fromSignal = fromBase();
    _toSignal = toBase();
  }
}

void Frame::modulateChannel(int channel_index) {
  FrameEngine &e = *_engines[channel_index];
  float delta = params[DELTA_PARAM].getValue();
  if (inputs[DELTA_INPUT].isConnected()) {
    float delta_port = clamp(inputs[DELTA_INPUT].getPolyVoltage(channel_index) / 5.0f, -5.0f, 5.0f);
    delta = clamp(delta_port + delta, -5.0f, 5.0f);
  }
  e.delta = delta;

  // TODO FIXME
  e.section_position = params[SECTION_PARAM].getValue() * (15);
  if (inputs[SECTION_INPUT].isConnected()) {
    e.section_position += clamp(inputs[SECTION_INPUT].getPolyVoltage(channel_index), -5.0f, 5.0f);
  }
  e.section_position = clamp(e.section_position, 0.0f, 15.0f);
}

int Frame::channels() {
  if (baseConnected()) {
    int input_channels = _fromSignal->channels;
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

  FrameEngine &e = *_engines[channel_index];

  if (inputs[CLK_INPUT].isConnected()) {
    e.use_ext_phase = true;
    e.ext_phase = clamp(inputs[CLK_INPUT].getPolyVoltage(channel_index) / 10, 0.0f, 1.0f);
  } else {
    e.use_ext_phase = false;
  }

  float in = _fromSignal->signal[channel_index];
  e.step(in, _sampleTime);

  float out = e.read();
  _toSignal->signal[channel_index] = out;
}

void Frame::updateLights(const ProcessArgs &args) {
  FrameEngine &e = *_engines[0];
  float phase = e.active_section->phase;

  lights[PHASE_LIGHT + 1].setSmoothBrightness(phase, _sampleTime * light_divider.getDivision());

  // FIXME
  float attenuation_power = 0.0f;

  if (!e.recording || !e.active_section) {
    lights[RECORD_MODE_LIGHT + 0].value = 0.0;
    lights[RECORD_MODE_LIGHT + 1].value = 0.0;
    lights[RECORD_MODE_LIGHT + 2].value = 0.0;
  } else if (e.active_section->mode == RecordMode::EXTEND) {
    lights[RECORD_MODE_LIGHT + 0].setSmoothBrightness(
        1.0f - attenuation_power, _sampleTime * light_divider.getDivision());
    lights[RECORD_MODE_LIGHT + 1].value = 0.0;
    lights[RECORD_MODE_LIGHT + 2].value = 0.0;
  } else if (e.active_section->mode == RecordMode::DUB) {
    lights[RECORD_MODE_LIGHT + 0].setSmoothBrightness(
        1.0f - attenuation_power, _sampleTime * light_divider.getDivision());
    lights[RECORD_MODE_LIGHT + 1].setSmoothBrightness(
        1.0f - attenuation_power, _sampleTime * light_divider.getDivision());
    lights[RECORD_MODE_LIGHT + 2].value = 0.0;
  } else if (e.active_section->mode == RecordMode::DEFINE_DIVISION_LENGTH) {
    lights[RECORD_MODE_LIGHT + 0].value = 0.0;
    lights[RECORD_MODE_LIGHT + 1].value = 0.0;
    lights[RECORD_MODE_LIGHT + 2].value = 1.0;
  }
}

void Frame::postProcessAlways(const ProcessArgs &args) {
  if (light_divider.process()) {
    updateLights(args);
  }
}

void Frame::addChannel(int channel_index) {
  _engines[channel_index] = new FrameEngine();
}

void Frame::removeChannel(int channel_index) {
  delete _engines[channel_index];
  _engines[channel_index] = nullptr;
}

struct FrameWidget : ModuleWidget {
  const int hp = 4;

  FrameWidget(Frame *module) {
    setModule(module);
    box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Frame.svg")));

		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<Rogan4PSGray>(mm2px(Vec(2.247, 18.399)), module, Frame::SECTION_PARAM));
		addParam(createParam<TL1105>(mm2px(Vec(7.365, 48.24)), module, Frame::PLAY_PARAM));
		addParam(createParam<TL1105>(mm2px(Vec(13.909, 48.28)), module, Frame::NEXT_PARAM));
		addParam(createParam<TL1105>(mm2px(Vec(0.848, 48.282)), module, Frame::PREV_PARAM));
		addParam(createParam<TL1105>(mm2px(Vec(3.485, 71.081)), module, Frame::UNDO_PARAM));
		addParam(createParam<TL1105>(mm2px(Vec(11.408, 71.22)), module, Frame::RECORD_MODE_PARAM));
		addParam(createParam<Rogan3PBlue>(mm2px(Vec(2.74, 81.455)), module, Frame::DELTA_PARAM));

		addInput(createInput<PJ301MPort>(mm2px(Vec(5.79, 34.444)), module, Frame::SECTION_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(5.79, 96.94)), module, Frame::DELTA_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(5.905, 109.113)), module, Frame::CLK_INPUT));

		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(8.528, 63.611)), module, Frame::PHASE_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(8.542, 67.836)), module, Frame::RECORD_MODE_LIGHT));

		// mm2px(Vec(18.593, 7.115))
		addChild(createWidget<Widget>(mm2px(Vec(0.758, 54.214))));
  }
};

Model *modelFrame = rack::createModel<Frame, FrameWidget>("Myrisa-Frame");
