#pragma once

#include "Member.hpp"

namespace kokopellivcv {

struct MemberWidget : ModuleWidget {
  const int hp = 3;

	MemberWidget(Member* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Member.svg")));

		addChild(createWidget<KokopelliScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<KokopelliScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	}
};

} // namespace kokopellivcv
