#include "Frame_IO.hpp"

extern Model *modelFrameX;

namespace myrisa {

struct FrameX : ExpanderModule<FrameExpanderMessage, MyrisaModule> {
	enum ParamIds {
		POS_PARAM,
		RATE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		POS_INPUT,
		RATE_INPUT,
		PREV_INPUT,
		PLAY_INPUT,
		NEXT_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	FrameX() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(POS_PARAM, 0.f, 1.f, 0.f, "");
		configParam(RATE_PARAM, 0.f, 1.f, 0.f, "");
    setBaseModelPredicate([](Model *m) { return m == modelFrame; });
	}

  void processAlways(const ProcessArgs& args) override;
};

} // namespace myrisa

