#pragma once

#include "Frame_IO.hpp"

using namespace std;

namespace myrisa {

struct Frame : ExpanderModule<SignalExpanderMessage, MyrisaModule> {
	enum ParamIds {
		SCENE_PARAM,
		PLAY_PARAM,
		NEXT_PARAM,
		PREV_PARAM,
		UNDO_PARAM,
		RECORD_MODE_PARAM,
		DELTA_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		SCENE_INPUT,
		DELTA_INPUT,
		CLK_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		DELTA_MODE_LIGHT,
		ENUMS(RECORD_MODE_LIGHT, 3),
		NUM_LIGHTS
	};

  enum Mode {
    ADD,
    EXTEND,
    READ
  };

	static constexpr int maxLayers = 64;
	static constexpr int numScenes = 16;
	static constexpr float recordThreshold = 0.05f;

  SignalExpanderMessage *_toSignal = NULL;
  SignalExpanderMessage *_fromSignal = NULL;

  struct Engine {
    struct Scene {
      struct Layer {
        vector<float> buffer;
        vector<float> attenuation_envelope;
        vector<int> attenuation_target_layers;
      };

      vector<Engine::Scene::Layer*> layers;
      unsigned int scene_length = 0;
      unsigned int phase = 0;
      int current_layer = -1;
      vector<int> selected_layers;
      Mode mode = Mode::READ;
    };

    float scene_position = 0.0f;
    float delta = 0.0f;
    bool recording = false;
    int scene_number_of_recording = 0;
    Scene* scenes[numScenes]{};
  };

  Engine *_engines[maxChannels]{};

  Frame() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(SCENE_PARAM, 0.f, 1.f, 0.f, "Scene");
		configParam(PLAY_PARAM, 0.f, 1.f, 0.f, "Play Layer");
		configParam(NEXT_PARAM, 0.f, 1.f, 0.f, "Next Layer");
		configParam(PREV_PARAM, 0.f, 1.f, 0.f, "Prev Layer");
		configParam(UNDO_PARAM, 0.f, 1.f, 0.f, "Undo");
		configParam(RECORD_MODE_PARAM, 0.f, 1.f, 0.f, "Record Mode");
		configParam(DELTA_PARAM, 0.f, 1.f, 0.5f, "Delta");

    // TODO chainable
    setBaseModelPredicate([](Model *m) { return m == modelSignal; });
 }

  void modulateChannel(int c) override;
  void addChannel(int c) override;
  void removeChannel(int c) override;
  void processAlways(const ProcessArgs &args) override;
  void processChannel(const ProcessArgs &args, int channel) override;
  void postProcessAlways(const ProcessArgs &args) override;
};

} // namespace myrisa
