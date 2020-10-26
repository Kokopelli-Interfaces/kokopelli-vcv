#include "plugin.hpp"
#include "widgets/knobs.hpp"

struct MyrisaFrameX : Module {
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

	MyrisaFrameX() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(POS_PARAM, 0.f, 1.f, 0.f, "");
		configParam(RATE_PARAM, 0.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct MyrisaFrameXWidget : ModuleWidget {
	MyrisaFrameXWidget(MyrisaFrameX* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/FrameX.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<RoganHalfPGray>(mm2px(Vec(2.968, 18.986)), module, MyrisaFrameX::POS_PARAM));
		addParam(createParam<RoganHalfPRed>(mm2px(Vec(2.968, 45.486)), module, MyrisaFrameX::RATE_PARAM));

		addInput(createInput<PJ301MPort>(mm2px(Vec(3.512, 30.498)), module, MyrisaFrameX::POS_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(3.512, 56.998)), module, MyrisaFrameX::RATE_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(3.512, 76.383)), module, MyrisaFrameX::PREV_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(3.512, 90.064)), module, MyrisaFrameX::PLAY_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(3.512, 103.272)), module, MyrisaFrameX::NEXT_INPUT));
	}
};


Model* modelMyrisaFrameX = createModel<MyrisaFrameX, MyrisaFrameXWidget>("MyrisaFrameX");
