#include "plugin.hpp"
#include "widgets/knobs.hpp"

struct MyrisaPlay : Module {
	enum ParamIds {
		OUT_POWER_PARAM,
		MIX_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		VEL_INPUT,
		GATE_INPUT,
		VOCT_INPUT,
		OUT_POWER_INPUT,
		MIX_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SEL_VEL_OUTPUT,
		SEL_GATE_OUTPUT,
		SEL_VOCT_OUTPUT,
		VEL_OUTPUT,
		GATE_OUTPUT,
		VOCT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	MyrisaPlay() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(OUT_POWER_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MIX_PARAM, 0.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct MyrisaPlayWidget : ModuleWidget {
	MyrisaPlayWidget(MyrisaPlay* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Play.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<RoganHalfPSRed>(mm2px(Vec(13.772, 71.608)), module, MyrisaPlay::OUT_POWER_PARAM));
		addParam(createParam<RoganHalfPSLightPurple>(mm2px(Vec(1.499, 71.633)), module, MyrisaPlay::MIX_PARAM));

		addInput(createInput<PJ301MPort>(mm2px(Vec(8.771, 17.295)), module, MyrisaPlay::VEL_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(3.788, 29.78)), module, MyrisaPlay::GATE_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(13.339, 29.78)), module, MyrisaPlay::VOCT_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(14.672, 82.791)), module, MyrisaPlay::OUT_POWER_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(2.398, 82.816)), module, MyrisaPlay::MIX_INPUT));

		addOutput(createOutput<PJ301MPort>(mm2px(Vec(8.444, 43.303)), module, MyrisaPlay::SEL_VEL_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.286, 55.431)), module, MyrisaPlay::SEL_GATE_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(13.582, 55.431)), module, MyrisaPlay::SEL_VOCT_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(8.444, 96.627)), module, MyrisaPlay::VEL_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.286, 108.576)), module, MyrisaPlay::GATE_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(13.582, 108.576)), module, MyrisaPlay::VOCT_OUTPUT));
	}
};


Model* modelMyrisaPlay = createModel<MyrisaPlay, MyrisaPlayWidget>("MyrisaPlay");
