#pragma once

#include "Aion.hpp"
#include "util/colors.hpp"

namespace kokopellivcv {

struct AionWidget : ModuleWidget {
  const int hp = 5;

	AionWidget(Aion* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Aion.svg")));

		addChild(createWidget<KokopelliScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<KokopelliScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<KokopelliScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<KokopelliScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<MediumLEDButton>(mm2px(Vec(-5.421, 13.815)), module, Aion::PREV_MOVEMENT_PARAM));

		// mm2px(Vec(3.974, 1.465))
		addChild(createWidget<Widget>(mm2px(Vec(-5.008, -29.668))));
		// mm2px(Vec(3.974, 1.465))
		addChild(createWidget<Widget>(mm2px(Vec(-6.154, -26.955))));
		// mm2px(Vec(6.594, 1.46))
		addChild(createWidget<Widget>(mm2px(Vec(-7.335, 4.937))));
		// mm2px(Vec(6.548, 1.46))
		addChild(createWidget<Widget>(mm2px(Vec(-7.32, 6.863))));
		// mm2px(Vec(3.974, 1.465))
		addChild(createWidget<Widget>(mm2px(Vec(-5.008, 17.334))));
		// mm2px(Vec(3.974, 1.465))
		addChild(createWidget<Widget>(mm2px(Vec(-6.154, 19.929))));
		// mm2px(Vec(5.329, 1.46))
		addChild(createWidget<Widget>(mm2px(Vec(-6.972, 22.758))));
	}
};

} // namespace kokopellivcv
