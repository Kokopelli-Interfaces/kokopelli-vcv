#include "Frame.hpp"

Frame::Frame() {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  configParam(LAYER_PARAM, 0.f, 1.f, 0.f, "Layer");
  configParam(SELECT_PARAM, 0.f, 1.f, 0.f, "Select");
  configParam(SCENE_PARAM, 0.f, 1.f, 0.f, "Scene");
  configParam(MODE_SWITCH_PARAM, 0.f, 1.f, 0.f, "Mode");
  configParam(LOOP_PARAM, 0.f, 1.f, 0.f, "Loop");
  configParam(RECORD_MODE_PARAM, 0.f, 1.f, 0.f, "Record Mode");
  configParam(RECORD_CONTEXT_PARAM, 0.f, 1.f, 0.f, "Record Context");
  configParam(DELTA_PARAM, 0.f, 1.f, 0.f, "Delta");

  setBaseModelPredicate([](Model *m) { return m == modelSignal; });
  light_divider.setDivision(16);
}

void Frame::sampleRateChange() {
  _sampleTime = APP->engine->getSampleTime();
  // FIXME
  // for (auto engine : _engines) {
  //   if (engine) {
  //     engine->_sample_time = _sampleTime;
  //   }
  // }
}

void Frame::processAlways(const ProcessArgs &args) {
  if (baseConnected()) {
    _from_signal = fromBase();
    _to_signal = toBase();
  }
}

inline RecordMode getEngineMode(float delta, float record_threshold, myrisa::dsp::frame::Engine *e) {
  if (delta < 0.50f - record_threshold) {
    return RecordMode::DUB;
  } else if (0.50f + record_threshold < delta) {
    return RecordMode::EXTEND;
  }

  return RecordMode::READ;
}

void Frame::modulateChannel(int channel_index) {
  myrisa::dsp::frame::Engine *e = _engines[channel_index];

  e->_use_ext_phase = inputs[CLK_INPUT].isConnected();

  float widget_delta = params[DELTA_PARAM].getValue();
  if (inputs[DELTA_INPUT].isConnected()) {
    float delta_port = inputs[DELTA_INPUT].getPolyVoltage(channel_index) / 10;
    widget_delta = rack::clamp(delta_port + widget_delta, 0.0f, 1.0f);
  }

  // taking to the power of 3 gives a more intuitive curve
  e->_delta.attenuation = rack::clamp(pow(widget_delta, 3), 0.0f, 1.0f);
  e->_delta.recording = _recordThreshold < widget_delta ? true : false;

  // TODO FIXME
  // delta.mode = RecordMode::CREATE;
  // delta.context = RecordContext::STRUCTURE;

  float scene_position = params[SCENE_PARAM].getValue() * (15);
  if (inputs[SCENE_INPUT].isConnected()) {
    scene_position += rack::clamp(inputs[SCENE_INPUT].getPolyVoltage(channel_index), -5.0f, 5.0f);
  }
  scene_position = rack::clamp(scene_position, 0.0f, 15.0f);
  e->updateScenePosition(scene_position);
}

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
        inputs[CLK_INPUT].getPolyVoltage(channel_index) / 10, 0.0f, 1.0f);
  }

  e->_in = _from_signal->signal[channel_index];
  e->step();
  _to_signal->signal[channel_index] = e->read();
}

void Frame::updateLights(const ProcessArgs &args) {
  myrisa::dsp::frame::Engine *e = _engines[0];

  if (e->_active_scene) {
    float phase = e->_active_scene->_phase;
    lights[PHASE_LIGHT + 1].setSmoothBrightness(
        phase, _sampleTime * light_divider.getDivision());
  }

  // FIXME
  float attenuation_power = 0.0f;

  if (e->_active_scene->isEmpty() && e->_mode == RecordMode::READ) {
    lights[RECORD_MODE_LIGHT + 0].value = 0.0;
    lights[RECORD_MODE_LIGHT + 1].value = 0.0;
    lights[RECORD_MODE_LIGHT + 2].value = 0.0;
  } else if (e->_mode == RecordMode::READ) {
    lights[RECORD_MODE_LIGHT + 0].value = 0.0;
    lights[RECORD_MODE_LIGHT + 1].value = 1.0;
    lights[RECORD_MODE_LIGHT + 2].value = 0.0;
  } else if (!e->_active_scene->_internal_phase_defined) {
    lights[RECORD_MODE_LIGHT + 0].value = 0.0;
    lights[RECORD_MODE_LIGHT + 1].value = 0.0;
    lights[RECORD_MODE_LIGHT + 2].value = 1.0;
  } else if (e->_mode == RecordMode::EXTEND) {
    lights[RECORD_MODE_LIGHT + 0].setSmoothBrightness(
        1.0f - attenuation_power, _sampleTime * light_divider.getDivision());
    lights[RECORD_MODE_LIGHT + 1].value = 0.0;
    lights[RECORD_MODE_LIGHT + 2].value = 0.0;
  } else if (e->_mode == RecordMode::DUB) {
    lights[RECORD_MODE_LIGHT + 0].setSmoothBrightness(
        1.0f - attenuation_power, _sampleTime * light_divider.getDivision());
    lights[RECORD_MODE_LIGHT + 1].setSmoothBrightness(
        1.0f - attenuation_power, _sampleTime * light_divider.getDivision());
    lights[RECORD_MODE_LIGHT + 2].value = 0.0;
  }
}

void Frame::postProcessAlways(const ProcessArgs &args) {
  if (light_divider.process()) {
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

struct FrameWidget : ModuleWidget {
  const int hp = 4;

  FrameWidget(Frame *module) {
    setModule(module);
    box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Frame.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


		addParam(createParam<Rogan1PGray>(mm2px(Vec(7.631, 18.94)), module, Frame::LAYER_PARAM));
		addParam(createParam<LEDButton>(mm2px(Vec(17.878, 25.239)), module, Frame::SELECT_PARAM));
		addParam(createParam<Rogan1HPSWhite>(mm2px(Vec(5.333, 35.894)), module, Frame::SCENE_PARAM));
		addParam(createParam<LEDButton>(mm2px(Vec(17.878, 48.367)), module, Frame::MODE_SWITCH_PARAM));
		addParam(createParam<LEDButton>(mm2px(Vec(9.715, 62.232)), module, Frame::LOOP_PARAM));
		addParam(createParam<LEDButton>(mm2px(Vec(16.926, 76.376)), module, Frame::RECORD_CONTEXT_PARAM));
		addParam(createParam<ToggleLEDButton>(mm2px(Vec(2.596, 76.389)), module, Frame::RECORD_MODE_PARAM));
		addParam(createParam<Rogan3PBlue>(mm2px(Vec(5.333, 82.157)), module, Frame::DELTA_PARAM));

		addInput(createInput<PJ301MPort>(mm2px(Vec(8.522, 51.114)), module, Frame::SCENE_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(8.384, 97.419)), module, Frame::DELTA_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(1.798, 109.491)), module, Frame::CLK_INPUT));

		addOutput(createOutput<PJ301MPort>(mm2px(Vec(15.306, 109.491)), module, Frame::PHASE_OUTPUT));

		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(19.309, 26.67)), module, Frame::SELECT_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(19.308, 49.797)), module, Frame::MODE_SWITCH_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(11.16, 63.676)), module, Frame::LOOP_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(11.181, 72.898)), module, Frame::STRUCTURE_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(18.357, 77.807)), module, Frame::RECORD_CONTEXT_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(4.027, 77.82)), module, Frame::RECORD_MODE_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(11.046, 111.955)), module, Frame::PHASE_LIGHT));

		// mm2px(Vec(10.219, 3.514))
		addChild(createWidget<Widget>(mm2px(Vec(7.591, 14.293))));
		// mm2px(Vec(10.029, 3.514))
		addChild(createWidget<Widget>(mm2px(Vec(7.645, 31.288))));
		// mm2px(Vec(5.195, 3.514))
		addChild(createWidget<Widget>(mm2px(Vec(2.599, 61.438))));
		// mm2px(Vec(5.195, 3.514))
		addChild(createWidget<Widget>(mm2px(Vec(17.838, 61.592))));
		// mm2px(Vec(5.195, 3.514))
		addChild(createWidget<Widget>(mm2px(Vec(2.599, 65.909))));
		// mm2px(Vec(5.195, 3.514))
		addChild(createWidget<Widget>(mm2px(Vec(17.838, 66.063))));
  }
};

Model *modelFrame = rack::createModel<Frame, FrameWidget>("Myrisa-Frame");
