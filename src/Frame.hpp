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

        bool fully_attenuated = false;
        bool attenuation_flag = false;

        vector<Engine::Scene::Layer*> target_layers;

        // depends on target layers and type
        Mode type;

        // TODO start offset for Extend mode is relative to division or target layers?
        // start & end offset for add mode are relative to max length in target layers
        // actually just start off with extend mode same as add mode
        unsigned int start_division = 0;
        unsigned int start_division_offset = 0;
        unsigned int end_division = 0;
        unsigned int end_division_offset = 0;
        unsigned int length = 0;

        float read(unsigned int phase, unsigned int division_length);
        float readAttenuation(unsigned int phase, unsigned int division_length);
      };

      vector<Engine::Scene::Layer*> layers;
      Engine::Scene::Layer *current_layer = NULL;
      vector<Engine::Scene::Layer*> selected_layers;

      // TODO 
      vector<float> scene_buffer;

      unsigned int scene_length = 0;

      // position from start of first layer to end of longest layer
      unsigned int phase = 0;
      Mode mode = Mode::READ;

      unsigned int getDivisionLength();
      void step(float in, float attenuation_power);
      float read(float attenuation_power);
    };

    float scene_position = 0.0f;
    float delta = 0.0f;
    bool recording = false;

    Scene* active_scene = NULL;
    Scene* recording_dest_scene = NULL;
    Scene* scenes[numScenes]{};

    void startRecording();
    void endRecording();
    void step(float in);
    float read();
    bool deltaEngaged();
  };

  static constexpr float recordThreshold = 0.05f;
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
