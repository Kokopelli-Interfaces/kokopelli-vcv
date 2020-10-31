#include "FrameX.hpp"

void FrameX::processAlways(const ProcessArgs& args) {
  return;
}

struct FrameXWidget : ModuleWidget {
	FrameXWidget(FrameX* module) {
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/FrameX.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<RoganHalfPGray>(mm2px(Vec(2.968, 18.986)), module, FrameX::POS_PARAM));
		addParam(createParam<RoganHalfPRed>(mm2px(Vec(2.968, 45.486)), module, FrameX::RATE_PARAM));

		addInput(createInput<PJ301MPort>(mm2px(Vec(3.512, 30.498)), module, FrameX::POS_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(3.512, 56.998)), module, FrameX::RATE_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(3.512, 76.383)), module, FrameX::PREV_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(3.512, 90.064)), module, FrameX::PLAY_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(3.512, 103.272)), module, FrameX::NEXT_INPUT));
	}
};

Model *modelFrameX = rack::createModel<FrameX, FrameXWidget>("Myrisa-FrameX");
