#pragma once

#include "Frame_IO.hpp"
#include "dsp/LFO.hpp"

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

  enum Mode { ADD, EXTEND, READ };

  static constexpr int numScenes = 16;

  SignalExpanderMessage *_toSignal = NULL;
  SignalExpanderMessage *_fromSignal = NULL;

  struct Engine {
    struct Scene {
      struct Layer {
        unsigned int start_division = 0;
        int divisions = 0;
        int length = 0;
        int samples_per_division = 0;
        int define_division_length = false;

        vector<vector<float>> division_buffers;
        vector<vector<float>> division_attenuation_sends;

        vector<Engine::Scene::Layer*> target_layers;

        bool fully_attenuated = false;
        bool attenuation_flag = false;

        void step(unsigned int phase, float in, float attenuation_power);
        void write(unsigned int division, float phase, float sample, float send_attenuation);
        void startRecording(vector<Engine::Scene::Layer *> selected_layers, unsigned int phase, unsigned int division_length);

        void addDivision();
        void endRecording(unsigned int phase, unsigned int division_length);
        float readAttenuation(unsigned int current_division, float phase);
        float readSample(unsigned int current_division, float phase);
        float readGeneric(unsigned int current_division, float phase, bool read_attenuation);
      };

      vector<Engine::Scene::Layer*> layers;
      Engine::Scene::Layer *current_layer = NULL;
      vector<Engine::Scene::Layer*> selected_layers;

      Mode mode = Mode::READ;
      int samples_per_division = 0;

      unsigned division = 0;

      LowFrequencyOscillator phase_oscillator;
      bool ext_phase = false;
      bool ext_phase_flipped = false;
      float last_ext_phase = 0.0f;

      void addLayer();
      void setMode(Mode new_mode, float sample_time);
      void setExtPhase(float ext_phase);
      Mode getMode();
      float getPhase();
      bool stepPhase(float sample_time);
      bool isEmpty();
      unsigned int getDivisionLength();
      void step(float in, float attenuation_power, float sample_time);
      void updateSceneLength();
      float read();
    };

    float scene_position = 0.0f;
    float delta = 0.0f;
    bool recording = false;

    Scene* active_scene = NULL;
    Scene* recording_dest_scene = NULL;
    Scene* scenes[numScenes]{};

    void startRecording(float sample_time);
    void endRecording(float sample_time);
    void step(float in, float sample_time);
    float read();
    bool deltaEngaged();
  };

  static constexpr float recordThreshold = 0.05f;

  float _sampleTime = 1.0f;
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

  void sampleRateChange() override;
  int channels() override;
  void modulateChannel(int c) override;
  void addChannel(int c) override;
  void removeChannel(int c) override;
  void processAlways(const ProcessArgs &args) override;
  void processChannel(const ProcessArgs &args, int channel) override;
  void postProcessAlways(const ProcessArgs &args) override;
  void updateLights(const ProcessArgs &args);
};

} // namespace myrisa
