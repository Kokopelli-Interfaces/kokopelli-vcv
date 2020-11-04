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

  int active_scene_i = round(e.scene_position * 16);
  Frame::Engine::Scene *active_scene = e.scenes[active_scene_i];
  if (!active_scene) {
    active_scene = new Engine::Scene();
    e.scenes[active_scene_i] = active_scene;
  }

  e.active_scene = active_scene;
}

// TODO, how to define division length?
// decided not to use clk port - maybe manual entry? 
// for now, size of first layer.
unsigned int Frame::Engine::Scene::getDivisionLength() {
  if (layers.size() > 0) {
    return layers[0]->buffer.size();
  }

  printf("no division yet\n");
  return 1;
}

void Frame::Engine::startRecording() {
  recording = true;
  recording_dest_scene = active_scene;


  if (delta > 0.50f + recordThreshold) {
    recording_dest_scene->mode = Mode::EXTEND;
  } else {
    recording_dest_scene->mode = Mode::ADD;
  }

  Engine::Scene::Layer *new_layer = new Engine::Scene::Layer();

  // TODO dependent on mode!
  unsigned int division_length = recording_dest_scene->getDivisionLength();
  unsigned int start_division_n = floor(recording_dest_scene->phase / division_length);
  int start_padding = recording_dest_scene->phase - division_length * start_division_n;

  printf("padding %d at start\n", start_padding);
  auto beginning = new_layer->buffer.begin();
  new_layer->buffer.insert(beginning, 0.0f, start_padding);

  // TODO round not floor
  // if (start_padding > 0) {
  //   new_layer->buffer.insert(beginning, 0.0f, start_padding);
  // } else {
  //   new_layer->buffer.erase(beginning + start_padding, beginning);
  // }

 new_layer->buffer.insert(beginning, 0.0f, start_padding);

  recording_dest_scene->layers.push_back(new_layer);
  recording_dest_scene->current_layer = new_layer;

  printf("start recording\n");
}

void Frame::Engine::endRecording() {
  recording = false;
  recording_dest_scene->mode = Mode::READ;
  Frame::Engine::Scene::Layer* recorded_layer = recording_dest_scene->current_layer;

  unsigned int unpadded_recording_length = recorded_layer->buffer.size();
  printf("unpadded length: %d\n", unpadded_recording_length);

  // TODO depend on mode
  unsigned int division_length = recording_dest_scene->getDivisionLength();
  printf("div length: %d\n", division_length);
  unsigned int end_division_n = round(unpadded_recording_length / division_length);
  if (end_division_n == 0) {
    end_division_n++;
  }

  int end_padding = division_length * end_division_n - unpadded_recording_length;
  printf("end_padding %d\n", end_padding);
  auto ending = recorded_layer->buffer.end();
  if (end_padding > 0) {
    recorded_layer->buffer.insert(ending, 0.0f, end_padding);
  } else {
    recorded_layer->buffer.erase(ending + end_padding, ending);
  }

  unsigned int recording_length = recorded_layer->buffer.size();

  printf("disengage -- length: %d #layers: %ld\n", recording_length,
         recording_dest_scene->layers.size());


  if (recording_length > recording_dest_scene->scene_length) {
    recording_dest_scene->scene_length = recording_length;
  }

  recording_dest_scene = NULL;
  printf("end recording\n");
}

bool Frame::Engine::deltaEngaged() {
  return (delta > 0.50f + recordThreshold || delta < 0.50f - recordThreshold);
}

float Frame::Engine::step() {
  // TODO weighted mix
  // TODO global phase for all scenes, or not, have config option
  active_scene->phase++;
  if (active_scene->phase >= active_scene->scene_length) {
    active_scene->phase = 0;
  }

  float out = 0.0f;

  for (auto layer : active_scene->layers) {
    // don't output what we just put in, avoids FB
    if (active_scene->mode != Mode::READ && layer == active_scene->current_layer) {
      continue;
    }

    // TODO attenuation envelopes
    if (layer->buffer.size() > 0) {
      unsigned int layer_sample_i =
          active_scene->phase % layer->buffer.size();
      out += layer->buffer[layer_sample_i];
    }
  }

  return out;
}

void Frame::processChannel(const ProcessArgs& args, int c) {
  Engine &e = *_engines[c];

  if (!e.recording && e.deltaEngaged()) {
    e.startRecording();
  }

  if (e.recording && !e.deltaEngaged()) {
    e.endRecording();
  }

  if (e.recording) {
    Engine::Scene *rec_scene = e.recording_dest_scene;
    Engine::Scene::Layer *rec_layer = rec_scene->current_layer;

    if (!e.deltaEngaged()) {
    } else if (rec_scene->mode == Mode::EXTEND) {
      // TODO attenuation
      float next_in = _fromSignal->signal[c];
      rec_layer->buffer.push_back(next_in);
    } else {
      // TODO
    }
  }

  float next_out = e.step();
  _toSignal->signal[c] = next_out;
}

void Frame::postProcessAlways(const ProcessArgs &args) {
  // TODO function set lights
  // TODO lightDivider similar to LFO.cpp
  bool channel_recording = false;
  for (int channel_i = 0; channel_i < 1; channel_i++) {
    Engine &e = *_engines[channel_i];
    if (e.recording) {
      channel_recording = true;

      if (e.active_scene->mode == Mode::EXTEND) {
        lights[RECORD_MODE_LIGHT + 0].value = 1.0;
        lights[RECORD_MODE_LIGHT + 2].value = 0.0;
      } else if (e.active_scene->mode == Mode::ADD) {
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
