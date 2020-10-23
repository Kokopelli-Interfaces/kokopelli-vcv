#include "plugin.hpp"
#include "widgets/knobs.hpp"

struct MyrisaSignal : Module {
	enum ParamIds {
		MIX_PARAM,
		VCA_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN_INPUT,
		MIX_INPUT,
		VCA_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SEL_OUTPUT,
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	MyrisaSignal() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(MIX_PARAM, 0.f, 1.f, 0.f, "");
		configParam(VCA_PARAM, 0.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct MyrisaSignalWidget : ModuleWidget {
	MyrisaSignalWidget(MyrisaSignal* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/MyrisaSignal.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<RoganHalfPSLightPurple>(mm2px(Vec(2.687, 50.584)), module, MyrisaSignal::MIX_PARAM));
		addParam(createParam<RoganHalfPSRed>(mm2px(Vec(2.687, 77.502)), module, MyrisaSignal::VCA_PARAM));

		addInput(createInput<PJ301MPort>(mm2px(Vec(3.69, 17.88)), module, MyrisaSignal::IN_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(3.586, 61.767)), module, MyrisaSignal::MIX_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(3.586, 88.685)), module, MyrisaSignal::VCA_INPUT));

		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.516, 32.077)), module, MyrisaSignal::SEL_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.516, 108.703)), module, MyrisaSignal::OUT_OUTPUT));
	}
};


Model* modelMyrisaSignal = createModel<MyrisaSignal, MyrisaSignalWidget>("MyrisaSignal");
