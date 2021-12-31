#pragma once

#include "kokopellivcv.hpp"
#include "widgets.hpp"

extern Model *modelAion;

namespace kokopellivcv {

struct Aion : KokopelliVcvModule {
	enum ParamId {
		PREV_MOVEMENT_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		INPUTS_LEN
	};
	enum OutputId {
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	Aion() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(PREV_MOVEMENT_PARAM, 0.f, 1.f, 0.f, "Previous Movement");
	}

  // void sampleRateChange() override;
  // int channels() override;
  // void modulate() override;
  // void modulateChannel(int c) override;
  // void addChannel(int c) override;
  // void removeChannel(int c) override;
  // void processAlways(const ProcessArgs &args) override;
  void processChannel(const ProcessArgs &args, int channel) override;
  // void postProcessAlways(const ProcessArgs &args) override;
  // void updateLights(const ProcessArgs &args);
  // void updateLight(int light, NVGcolor color, float strength);
};

} // namespace kokopellivcv

