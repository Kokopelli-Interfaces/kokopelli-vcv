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
		ENUMS(PHASE_LIGHT, 3),
		ENUMS(RECORD_MODE_LIGHT, 3),
		NUM_LIGHTS
	};

  enum Mode {
    ADD,
    EXTEND,
    READ
  };

	static constexpr int numScenes = 16;

  SignalExpanderMessage *_toSignal = NULL;
  SignalExpanderMessage *_fromSignal = NULL;

  struct Engine {
    struct Scene {
      struct Layer {
        vector<float> buffer;
        vector<float> attenuation_envelope;
        vector<int> attenuation_target_layers;
        // TODO
        unsigned int skip_samples;

        float readSample(float phase);
      };

      vector<Engine::Scene::Layer*> layers;
      Engine::Scene::Layer *current_layer = NULL;
      vector<Engine::Scene::Layer*> selected_layers;

      unsigned int scene_length = 0;

      unsigned int phase = 0;
      Mode mode = Mode::READ;

      unsigned int getDivisionLength();
    };

    static constexpr float recordThreshold = 0.05f;

    float scene_position = 0.0f;
    float delta = 0.0f;
    bool recording = false;
    float attenuation_power = 0.0f;

    Scene* active_scene = NULL;
    Scene* recording_dest_scene = NULL;
    Scene* scenes[numScenes]{};

    void startRecording();
    void endRecording();
    bool deltaEngaged();
    float step();
  };

  Engine *_engines[maxChannels]{};
  dsp::ClockDivider lightDivider;

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
    lightDivider.setDivision(16);
 }

  void modulateChannel(int c) override;
  void addChannel(int c) override;
  void removeChannel(int c) override;
  void processAlways(const ProcessArgs &args) override;
  void processChannel(const ProcessArgs &args, int channel) override;
  void postProcessAlways(const ProcessArgs &args) override;
  void updateLights(const ProcessArgs &args);
};

} // namespace myrisa
