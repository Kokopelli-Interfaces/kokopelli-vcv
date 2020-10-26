#include "plugin.hpp"
#include "widgets/knobs.hpp"

struct Myrisa4Signal : Module {
	enum ParamIds {
		VCA_PARAM,
		MIX_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN_1_INPUT,
		IN_2_INPUT,
		IN_3_INPUT,
		IN_4_INPUT,
		VCA_INPUT,
		MIX_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SEL_1_OUTPUT,
		SEL_2_OUTPUT,
		SEL_3_OUTPUT,
		SEL_4_OUTPUT,
		OUT_1_OUTPUT,
		OUT_2_OUTPUT,
		OUT_3_OUTPUT,
		OUT_4_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Myrisa4Signal() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(VCA_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MIX_PARAM, 0.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct Myrisa4SignalWidget : ModuleWidget {
	Myrisa4SignalWidget(Myrisa4Signal* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/4Signal.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<RoganHalfPSRed>(mm2px(Vec(13.772, 71.584)), module, Myrisa4Signal::VCA_PARAM));
		addParam(createParam<RoganHalfPSLightPurple>(mm2px(Vec(1.499, 71.609)), module, Myrisa4Signal::MIX_PARAM));

		addInput(createInput<PJ301MPort>(mm2px(Vec(4.04, 18.394)), module, Myrisa4Signal::IN_1_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(12.859, 18.394)), module, Myrisa4Signal::IN_2_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(4.04, 27.468)), module, Myrisa4Signal::IN_3_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(12.859, 27.468)), module, Myrisa4Signal::IN_4_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(14.672, 82.768)), module, Myrisa4Signal::VCA_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(2.398, 82.792)), module, Myrisa4Signal::MIX_INPUT));

		addOutput(createOutput<PJ301MPort>(mm2px(Vec(4.04, 44.348)), module, Myrisa4Signal::SEL_1_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(12.926, 44.348)), module, Myrisa4Signal::SEL_2_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(4.04, 53.173)), module, Myrisa4Signal::SEL_3_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(12.926, 53.173)), module, Myrisa4Signal::SEL_4_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(4.04, 99.44)), module, Myrisa4Signal::OUT_1_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(12.926, 99.44)), module, Myrisa4Signal::OUT_2_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(4.04, 108.576)), module, Myrisa4Signal::OUT_3_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(12.926, 108.576)), module, Myrisa4Signal::OUT_4_OUTPUT));
	}
};


Model* modelMyrisa4Signal = createModel<Myrisa4Signal, Myrisa4SignalWidget>("Myrisa4Signal");
