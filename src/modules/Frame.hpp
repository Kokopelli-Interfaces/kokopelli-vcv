#pragma once

#include "Frame_interface.hpp"
#include "dsp/FrameEngine/FrameEngine.hpp"

using namespace std;
using namespace myrisa::dsp;

namespace myrisa {

struct Frame : ExpanderModule<SignalExpanderMessage, MyrisaModule> {
	enum ParamIds {
		SECTION_PARAM,
		PLAY_PARAM,
		NEXT_PARAM,
		PREV_PARAM,
		UNDO_PARAM,
		RECORD_MODE_PARAM,
		DELTA_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		SECTION_INPUT,
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

  SignalExpanderMessage *_toSignal = NULL;
  SignalExpanderMessage *_fromSignal = NULL;

  static constexpr float record_threshold = 0.05f;
  float _sampleTime = 1.0f;

  array<FrameEngine*, maxChannels> _engines;
  rack::dsp::ClockDivider light_divider;

  Frame();

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

