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
  // FIXME
  // for (auto engine : _engines) {
  //   if (engine) {
  //     engine->_sample_time = _sampleTime;
  //   }
  // }
}

void Frame::processAlways(const ProcessArgs &args) {
  if (baseConnected()) {
    _fromSignal = fromBase();
    _toSignal = toBase();
  }
}

inline float getAttenuationPower(float delta, float recording_threshold) {
  float read_position = 0.50f;
  float linear_attenuation_power;
  if (delta < read_position - recording_threshold) {
    linear_attenuation_power = read_position - recording_threshold - delta;
  } else if (delta > read_position - recording_threshold) {
    linear_attenuation_power = delta - read_position + recording_threshold;
  } else {
    linear_attenuation_power = 0.0f;
  }

  float range = read_position - recording_threshold;
  float linear_attenuation_power_scaled = linear_attenuation_power / range;

  // I found that taking to the power 3 gives the most intuitive attenuation
  // power curve
  float attenuation_power = rack::clamp(pow(linear_attenuation_power_scaled, 3), 0.0f, 1.0f);
  return attenuation_power;
}

inline RecordMode getEngineMode(float delta, float record_threshold, FrameEngine *e) {
  if (delta < 0.50f - record_threshold) {
    return RecordMode::DUB;
  } else if (0.50f + record_threshold < delta) {
    return RecordMode::EXTEND;
  }

  return RecordMode::READ;
}

void Frame::modulateChannel(int channel_index) {
  FrameEngine *e = _engines[channel_index];

  e->_use_ext_phase = inputs[CLK_INPUT].isConnected();

  float delta = params[DELTA_PARAM].getValue();
  if (inputs[DELTA_INPUT].isConnected()) {
    float delta_port = rack::clamp(inputs[DELTA_INPUT].getPolyVoltage(channel_index) / 5.0f, -5.0f, 5.0f);
    delta = rack::clamp(delta_port + delta, -5.0f, 5.0f);
  }
  e->_mode = getEngineMode(delta, recordThreshold, e);
  e->_attenuation = getAttenuationPower(delta, recordThreshold);

  float section_position = params[SECTION_PARAM].getValue() * (15);
  if (inputs[SECTION_INPUT].isConnected()) {
    section_position += rack::clamp(inputs[SECTION_INPUT].getPolyVoltage(channel_index), -5.0f, 5.0f);
  }
  section_position = rack::clamp(section_position, 0.0f, 15.0f);
  e->updateSectionPosition(section_position);
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

  FrameEngine *e = _engines[channel_index];

  if (e->_use_ext_phase) {
    e->_ext_phase = rack::clamp(
        inputs[CLK_INPUT].getPolyVoltage(channel_index) / 10, 0.0f, 1.0f);
  }

  e->_in = _fromSignal->signal[channel_index];
  e->step();
  _toSignal->signal[channel_index] = e->read();
}

void Frame::updateLights(const ProcessArgs &args) {
  FrameEngine *e = _engines[0];

  if (e->_active_section) {
    float phase = e->_active_section->_phase;
    lights[PHASE_LIGHT + 1].setSmoothBrightness(
        phase, _sampleTime * light_divider.getDivision());
  }

  // FIXME
  float attenuation_power = 0.0f;

  if (e->_active_section->isEmpty() && e->_mode == RecordMode::READ) {
    lights[RECORD_MODE_LIGHT + 0].value = 0.0;
    lights[RECORD_MODE_LIGHT + 1].value = 0.0;
    lights[RECORD_MODE_LIGHT + 2].value = 0.0;
  } else if (e->_mode == RecordMode::READ) {
    lights[RECORD_MODE_LIGHT + 0].value = 0.0;
    lights[RECORD_MODE_LIGHT + 1].value = 1.0;
    lights[RECORD_MODE_LIGHT + 2].value = 0.0;
  } else if (!e->_active_section->_phase_defined) {
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
  _engines[channel_index] = new FrameEngine();
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
