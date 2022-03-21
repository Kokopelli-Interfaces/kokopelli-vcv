#pragma once

#include "Ether.hpp"

namespace kokopellivcv {

struct EtherWidget : ModuleWidget {
  const int hp = 3;

	EtherWidget(Ether* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Ether.svg")));

		addChild(createWidget<KokopelliScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<KokopelliScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	}
};

} // namespace kokopellivcv
