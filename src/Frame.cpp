#include "Frame.hpp"

void Frame::processAlways(const ProcessArgs &args) {
  if (baseConnected()) {
    _fromSignal = fromBase();
    _toSignal = toBase();
  }
}

void Frame::modulateChannel(int c) {
  Engine &e = *_engines[c];
  e.delta = params[DELTA_PARAM].getValue();
  if (inputs[DELTA_INPUT].isConnected()) {
    e.delta *= clamp(inputs[DELTA_INPUT].getPolyVoltage(c) / 10.0f, 0.0f, 1.0f);
  }

  e.scene_position = params[SCENE_PARAM].getValue();
  if (inputs[SCENE_INPUT].isConnected()) {
    e.scene_position *= clamp(inputs[SCENE_INPUT].getPolyVoltage(c) / 10.0f, 0.0f, 1.0f);
  }
}


void Frame::processChannel(const ProcessArgs& args, int c) {
  Engine &e = *_engines[c];

  int active_scene_i = round(e.scene_position * 16);
  Engine::Scene *active_scene = e.scenes[active_scene_i];
  if (!active_scene) {
    e.scenes[active_scene_i] = new Engine::Scene();
    active_scene = e.scenes[active_scene_i];
  }

  // Record Engage
  if (!e.recording && (e.delta > 0.50f + recordThreshold || e.delta < 0.50f - recordThreshold)) {
    e.recording = true;
    e.scene_number_of_recording = active_scene_i;

    Engine::Scene::Layer *new_layer = new Engine::Scene::Layer();
    active_scene->layers.push_back(new_layer);
    active_scene->current_layer = active_scene->layers.size() - 1;

    if (e.delta > 0.50f + recordThreshold) {
      active_scene->mode = Mode::EXTEND;
    } else {
      active_scene->mode = Mode::ADD;
    }
  }

  if (e.recording) {
    Engine::Scene *rec_scene = e.scenes[e.scene_number_of_recording];
    Engine::Scene::Layer *current_layer = rec_scene->layers[rec_scene->current_layer];

    bool recording_disengaged = !(e.delta > 0.50f + recordThreshold || e.delta < 0.50f - recordThreshold);

    // Record Disengage
    if (recording_disengaged) {
        e.recording = false;
        rec_scene->mode = Mode::READ;

        unsigned int recording_length = current_layer->buffer.size();

        printf("disengages: %d %d #layer: %d rec_lery: %d\n", recording_length,
               rec_scene->scene_length, rec_scene->layers.size(),
               rec_scene->current_layer);

        if (recording_length > rec_scene->scene_length) {
          printf("length!: %d\n", recording_length);
          rec_scene->scene_length = recording_length;
        }
    } else if (rec_scene->mode == Mode::EXTEND) {
      // TODO attenuation
      current_layer->buffer.push_back(_fromSignal->signal[c]);
    } else {
      // TODO
    }
  }

  // TODO weighted mix
  // TODO global phase for all scenes, or not,have config option
  active_scene->phase++;
  if (active_scene->phase == active_scene->scene_length) {
    active_scene->phase = 0;
  }

  // TODO function Output next step
  float out = 0.0f;
  for (unsigned int layer_i = 0; layer_i < active_scene->layers.size(); layer_i++) {
    // don't output what we just put in, avoids FB
    if (active_scene->mode != Mode::READ && layer_i == active_scene->current_layer) {
      continue;
    }

    // TODO attenuation envelopes
    Engine::Scene::Layer *layer = active_scene->layers[layer_i];

    if (layer->buffer.size() > 0) {
      unsigned int layer_sample_i = active_scene->phase % layer->buffer.size();
      out += layer->buffer[layer_sample_i];
    }
  }

  _toSignal->signal[c] = out;
}

void Frame::postProcessAlways(const ProcessArgs &args) {
  // TODO function set lights
  // TODO lightDivider similar to LFO.cpp
  bool channel_recording = false;
  for (int channel_i = 0; channel_i < 1; channel_i++) {
    Engine &e = *_engines[channel_i];
    if (e.recording) {
      channel_recording = true;

      int active_scene_i = round(e.scene_position * 16);
      Mode active_scene_mode = e.scenes[active_scene_i]->mode;
      if (active_scene_mode == Mode::EXTEND) {
        lights[RECORD_MODE_LIGHT + 0].value = 1.0;
        lights[RECORD_MODE_LIGHT + 2].value = 0.0;
      } else if (active_scene_mode == Mode::ADD) {
        lights[RECORD_MODE_LIGHT + 0].value = 0.0;
        lights[RECORD_MODE_LIGHT + 2].value = 1.0;
      }

      break;
    }
  }

  if (!channel_recording) {
    lights[RECORD_MODE_LIGHT + 0].value = 0.0;
    lights[RECORD_MODE_LIGHT + 2].value = 0.0;
  }
}

void Frame::addChannel(int c) {
    _engines[c] = new Engine(); }

void Frame::removeChannel(int c) {
  delete _engines[c];
  _engines[c] = NULL;
}

struct FrameWidget : ModuleWidget {
  static constexpr int hp = 4;

  FrameWidget(Frame *module) {
    setModule(module);
    box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Frame.svg")));

		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<Rogan4PSGray>(mm2px(Vec(2.247, 18.399)), module, Frame::SCENE_PARAM));
		addParam(createParam<TL1105>(mm2px(Vec(7.365, 48.24)), module, Frame::PLAY_PARAM));
		addParam(createParam<TL1105>(mm2px(Vec(13.909, 48.28)), module, Frame::NEXT_PARAM));
		addParam(createParam<TL1105>(mm2px(Vec(0.848, 48.282)), module, Frame::PREV_PARAM));
		addParam(createParam<TL1105>(mm2px(Vec(3.485, 71.081)), module, Frame::UNDO_PARAM));
		addParam(createParam<TL1105>(mm2px(Vec(11.408, 71.22)), module, Frame::RECORD_MODE_PARAM));
		addParam(createParam<Rogan3PBlue>(mm2px(Vec(2.74, 81.455)), module, Frame::DELTA_PARAM));

		addInput(createInput<PJ301MPort>(mm2px(Vec(5.79, 34.444)), module, Frame::SCENE_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(5.79, 96.94)), module, Frame::DELTA_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(5.905, 109.113)), module, Frame::CLK_INPUT));

		addChild(createLight<MediumLight<GreenLight>>(mm2px(Vec(8.528, 63.611)), module, Frame::DELTA_MODE_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(8.542, 67.836)), module, Frame::RECORD_MODE_LIGHT));

		// mm2px(Vec(18.593, 7.115))
		addChild(createWidget<Widget>(mm2px(Vec(0.758, 54.214))));
  }
};

Model *modelFrame = rack::createModel<Frame, FrameWidget>("Myrisa-Frame");
