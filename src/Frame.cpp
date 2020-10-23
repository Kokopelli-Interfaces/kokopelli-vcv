#include "plugin.hpp"
#include "widgets/knobs.hpp"

struct MyrisaFrame : Module {
	enum ParamIds {
		SCENE_PARAM,
		PLAY_PARAM,
		RIGHT_PARAM,
		LEFT_PARAM,
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
		DELTA_MODE_LIGHT,
		RECORD_MODE_LIGHT,
		NUM_LIGHTS
	};

	MyrisaFrame() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(SCENE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(PLAY_PARAM, 0.f, 1.f, 0.f, "");
		configParam(RIGHT_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LEFT_PARAM, 0.f, 1.f, 0.f, "");
		configParam(UNDO_PARAM, 0.f, 1.f, 0.f, "");
		configParam(RECORD_MODE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(DELTA_PARAM, 0.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct MyrisaFrameWidget : ModuleWidget {
	MyrisaFrameWidget(MyrisaFrame* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/MyrisaFrame.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<Rogan4PSGray>(mm2px(Vec(2.247, 18.399)), module, MyrisaFrame::SCENE_PARAM));
		addParam(createParam<TL1105>(mm2px(Vec(7.365, 48.24)), module, MyrisaFrame::PLAY_PARAM));
		addParam(createParam<TL1105>(mm2px(Vec(13.909, 48.28)), module, MyrisaFrame::RIGHT_PARAM));
		addParam(createParam<TL1105>(mm2px(Vec(0.848, 48.282)), module, MyrisaFrame::LEFT_PARAM));
		addParam(createParam<TL1105>(mm2px(Vec(3.485, 71.081)), module, MyrisaFrame::UNDO_PARAM));
		addParam(createParam<TL1105>(mm2px(Vec(11.408, 71.22)), module, MyrisaFrame::RECORD_MODE_PARAM));
		addParam(createParam<Rogan3PBlue>(mm2px(Vec(2.74, 81.455)), module, MyrisaFrame::DELTA_PARAM));

		addInput(createInput<PJ301MPort>(mm2px(Vec(5.79, 34.444)), module, MyrisaFrame::SCENE_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(5.79, 96.94)), module, MyrisaFrame::DELTA_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(5.905, 109.113)), module, MyrisaFrame::CLK_INPUT));

		addChild(createLight<MediumLight<RedLight>>(mm2px(Vec(8.528, 63.611)), module, MyrisaFrame::DELTA_MODE_LIGHT));
		addChild(createLight<MediumLight<RedLight>>(mm2px(Vec(8.542, 67.836)), module, MyrisaFrame::RECORD_MODE_LIGHT));

		// mm2px(Vec(18.593, 7.115))
		addChild(createWidget<Widget>(mm2px(Vec(0.758, 54.214))));
	}
};


Model* modelMyrisaFrame = createModel<MyrisaFrame, MyrisaFrameWidget>("MyrisaFrame");
