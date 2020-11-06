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
  e.scene_position *= numScenes;

  int active_scene_i = round(e.scene_position);
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
  if (layers.size() > 0  && layers[0]->buffer.size() > 0) {
    return layers[0]->buffer.size();
  }

  return 1;
}

bool Frame::Engine::deltaEngaged() {
  return (delta > 0.50f + recordThreshold || delta < 0.50f - recordThreshold);
}

void Frame::Engine::startRecording() {
  recording = true;
  recording_dest_scene = active_scene;

  if (delta > 0.50f + recordThreshold || recording_dest_scene->layers.size() == 0) {
    recording_dest_scene->mode = Mode::EXTEND;
  } else {
    recording_dest_scene->mode = Mode::ADD;
  }

  Engine::Scene::Layer *new_layer = new Engine::Scene::Layer();

  // TODO dependent?
  unsigned int division_length = recording_dest_scene->getDivisionLength();

  new_layer->start_division = floor(recording_dest_scene->phase / division_length);
  new_layer->start_division_offset = recording_dest_scene->phase - new_layer->start_division * division_length;

  for (auto selected_layer : recording_dest_scene->selected_layers) {
    if (selected_layer) {
      new_layer->target_layers.push_back(selected_layer);
    } else {
      printf("No selected layers! error!");
    }
  }

  recording_dest_scene->layers.push_back(new_layer);
  recording_dest_scene->current_layer = new_layer;

  printf("Start recording\n");
}


int gcd(int a, int b)
{
    for (;;)
    {
        if (a == 0) return b;
        b %= a;
        if (b == 0) return a;
        a %= b;
    }
}

int lcm(int a, int b)
{
    int temp = gcd(a, b);

    return temp ? (a / temp * b) : 0;
}


// TODO delete layers that are now erased
void Frame::Engine::endRecording() {
  recording = false;
  recording_dest_scene->mode = Mode::READ;
  Frame::Engine::Scene::Layer* recorded_layer = recording_dest_scene->current_layer;

  printf("END recording\n");

  printf("-- recording buffer length: %ld\n", recorded_layer->buffer.size());

  // TODO depend on mode
  unsigned int division_length = recording_dest_scene->getDivisionLength();
  printf("-- div length: %d\n", division_length);

  recorded_layer->end_division = round(recording_dest_scene->phase / division_length);
  if (recorded_layer->end_division == 0) {
    recorded_layer->end_division = 1;
  }

  recorded_layer->end_division_offset = recording_dest_scene->phase - recorded_layer->end_division * division_length;

  unsigned int recording_length = recorded_layer->end_division - recorded_layer->start_division;
  if (recording_length > recording_dest_scene->scene_length) {
    vector<int> lengths;
    for (auto layer : recording_dest_scene->layers) {
      lengths.push_back(layer->end_division - layer->start_division);
    }
    int lc_multiple = accumulate(lengths.begin(), lengths.end(), 1, lcm);
    recording_dest_scene->scene_length = lc_multiple;
  }

  printf("-- length: %d #layers: %ld scene_size:%d\n", recording_length,
         recording_dest_scene->layers.size(), recording_dest_scene->scene_length);
  printf("-- start_div: %d, start_division_offset: %d\n", recorded_layer->start_division, recorded_layer->start_division_offset);
  printf("-- end_div: %d, end_division_offset: %d\n", recorded_layer->end_division, recorded_layer->end_division_offset);

  recording_dest_scene->selected_layers.push_back(recorded_layer);

  recording_dest_scene = NULL;
}


// TODO customizable respoonse?
// TODO attenuation clk divider, get smooth value
float getAttenuationPower(float delta, float recording_threshold) {
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

  // by taking to the power of 5, it is easier to create recordings that have no attenuation
  float attenuation_power =
      clamp(pow(linear_attenuation_power_scaled, 4), 0.0f, 1.0f);
  return attenuation_power;
}

void Frame::Engine::step(float in) {
  // TODO weighted mix
  // TODO global phase for all scenes, or not, have config option
  float attenuation_power = 0.0f;
  if (recording) {
    attenuation_power = getAttenuationPower(delta, recordThreshold);
  }

  for (auto scene : scenes) {
    if (scene) {
      if ((scene->mode == Mode::EXTEND || scene->mode == Mode::ADD) && scene->current_layer) {
        // TODO only if input present
        scene->current_layer->buffer.push_back(in);
        // TODO attenuation divider, don't need to save every point
        scene->current_layer->attenuation_envelope.push_back(attenuation_power);
      }

      scene->phase++;

      bool scene_end = scene->phase / scene->getDivisionLength() >= scene->scene_length;
      if (scene_end && scene->mode != Mode::EXTEND) {
        if (scene->mode == Mode::ADD) {
          endRecording();
        }

        scene->phase = 0;

        if (scene->mode == Mode::ADD) {
          startRecording();
        }
      }
    }
  }
}

 float Frame::Engine::Scene::Layer::read(unsigned int phase, unsigned int division_length) {
  float out = 0.0f;
  unsigned int layer_length = (end_division - start_division) * division_length;

  if (buffer.size() > 0) {
    // TODO smooth edges so no clicks
    // FIXME how to handle offset?
    unsigned int layer_phase = phase % layer_length;

    if (start_division_offset <= layer_phase && layer_phase <= layer_length + end_division_offset) {
      unsigned int layer_sample_i = layer_phase - start_division_offset;
      out = buffer[layer_sample_i];
    }
  }

  return out;
}

float Frame::Engine::Scene::Layer::readAttenuation(unsigned int phase) {
  float out = 0.0f;
  if (attenuation_envelope.size() > 0) {
    // TODO use start offsets and end offets
    // TODO smooth edges so no clicks
    unsigned int layer_sample_i = phase % attenuation_envelope.size();
    out = attenuation_envelope[layer_sample_i];
  }

  return out;
}

float Frame::Engine::Scene::read() {
  float out = 0.0f;
  for (auto layer : layers) {
    // don't output what we are writing to avoid FB in case of self-routing
    bool layer_is_recording = (mode != Mode::READ && layer == current_layer);
    if (layer_is_recording) {
      continue;
    }

    float layer_out = layer->read(phase, getDivisionLength());

    // FIXME
    float layer_attenuation = 0.0f;
    for (auto other_layer : layers) {
      if (layer == other_layer) {
        continue;
      }

      for (auto target_layer : other_layer->target_layers) {
        if (target_layer == layer) {
          layer_attenuation += other_layer->readAttenuation(phase);
          break;
        }
      }
    }

    // TODO attenuation envelopes
    layer_attenuation = clamp(layer_attenuation, 0.0f, 1.0f);
    layer_out *= (1 - layer_attenuation);
    out += layer_out;
  }

  return out;
}

float Frame::Engine::read() {
  float out = 0.0f;

  int scene_1 = floor(scene_position);
  int scene_2 = ceil(scene_position);
  float weight = scene_position - floor(scene_position);

  if (scenes[scene_1]) {
    out += scenes[scene_1]->read() * (1 - weight);
  }

  if (scenes[scene_2]) {
    out += scenes[scene_2]->read() * (weight);
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

  // TODO
  float next_in = _fromSignal->signal[c];
  e.step(next_in);
  float next_out = e.read();
  _toSignal->signal[c] = next_out;
}

void Frame::updateLights(const ProcessArgs &args) {
  Engine &e = *_engines[0];
  float position = 0.0f;
  if (e.active_scene->scene_length > 0) {
    position = (float)(e.active_scene->phase) / (float)(e.active_scene->scene_length * e.active_scene->getDivisionLength());
  }

  lights[PHASE_LIGHT + 1].setSmoothBrightness(position, args.sampleTime * lightDivider.getDivision());

  float attenuation_power = getAttenuationPower(e.delta, recordThreshold);

  if (!e.recording) {
    lights[RECORD_MODE_LIGHT + 0].value = 0.0;
    lights[RECORD_MODE_LIGHT + 2].value = 0.0;
  } else if (e.active_scene->mode == Mode::EXTEND) {
    lights[RECORD_MODE_LIGHT + 0].setSmoothBrightness(
        1.0f - attenuation_power, args.sampleTime * lightDivider.getDivision());
    lights[RECORD_MODE_LIGHT + 2].value = 0.0;
  } else if (e.active_scene->mode == Mode::ADD) {
    lights[RECORD_MODE_LIGHT + 0].value = 0.0;
    lights[RECORD_MODE_LIGHT + 2].setSmoothBrightness(
        1.0f - attenuation_power,
        args.sampleTime * lightDivider.getDivision());
  } else if (e.active_scene->mode == Mode::READ) {
    lights[RECORD_MODE_LIGHT + 0].value = 1.0;
    lights[RECORD_MODE_LIGHT + 2].value = 1.0;
  }
}

void Frame::postProcessAlways(const ProcessArgs &args) {
  if (lightDivider.process()) {
    updateLights(args);
  }
}

void Frame::addChannel(int c) {
    _engines[c] = new Engine();
}

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

		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(8.528, 63.611)), module, Frame::PHASE_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(8.542, 67.836)), module, Frame::RECORD_MODE_LIGHT));

		// mm2px(Vec(18.593, 7.115))
		addChild(createWidget<Widget>(mm2px(Vec(0.758, 54.214))));
  }
};

Model *modelFrame = rack::createModel<Frame, FrameWidget>("Myrisa-Frame");
