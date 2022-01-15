#pragma once

#include "Aion.hpp"
#include "util/colors.hpp"

namespace kokopellivcv {

struct AionTextBox : TextBox {
	Aion *_module = nullptr;
	int _previous_displayed_value = -1;

	AionTextBox(Aion *m, NVGcolor background_color, NVGcolor text_color, rack::math::Vec pos, rack::math::Vec size) : TextBox() {
    _module = m;
    fontPath = "res/fonts/Nunito-Bold.ttf";
    font_size = 12;
    letter_spacing = 0.f;
    backgroundColor = background_color;
    textColor = text_color;
    box.pos = pos;
    box.size = size;
    textOffset = Vec(box.size.x * 0.5f, 0.f);
    // textAlign = NVG_ALIGN_CENTER | NVG_ALIGN_TOP;
  }

  // void step() override {
  //   TextBox::step();
		// if(_module) {
    //   if (_module->channels() == 0) {
    //     return;
    //   }
    //   // kokopellivcv::dsp::aion::Engine* e = _module->_engines[0];
    //   // update(e);
    // }
  // }

  // virtual void update(kokopellivcv::dsp::circle::Engine* engine) {
  //   return;
  // };

	void setDisplayValue(int v) {
		std::string s;
		if(v != _previous_displayed_value) {
			_previous_displayed_value = v;
      s = string::f("%d", v);
      setText(s);
		}
	}
};


struct AionWidget : ModuleWidget {
  const int hp = 4;

  AionTextBox *_aion_box;
  AionTextBox *_song_box;

  AionTextBox *_next_next_movement_box;
  AionTextBox *_next_movement_box;
  AionTextBox *_current_movement_box;
  AionTextBox *_prev_movement_box;
  AionTextBox *_prev_prev_movement_box;

	AionWidget(Aion* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Aion.svg")));

		addChild(createWidget<KokopelliScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<KokopelliScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<KokopelliScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<KokopelliScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<MediumLEDButton>(mm2px(Vec(8.183, 41.077)), module, Aion::PREV_MOVEMENT_PARAM));

    auto top_box_size = mm2px(Vec(19.472, 4.312));
    _aion_box = new AionTextBox(module, colors::BOX_BG_DARK, colors::OBSERVED_SUN, mm2px(Vec(2.531, 14.862)), top_box_size);
    _song_box = new AionTextBox(module, colors::BOX_BG_DARK, colors::OBSERVED_SUN, mm2px(Vec(2.531, 20.550)), top_box_size);

    auto movement_box_size = mm2px(Vec(11.735, 4.327));

		_next_next_movement_box = new AionTextBox(module, colors::BOX_BG_DARK, colors::OBSERVED_SUN, mm2px(Vec(9.403, 51.468)), movement_box_size);
    _next_movement_box = new AionTextBox(module, colors::BOX_BG_DARK, colors::OBSERVED_SUN, mm2px(Vec(6.020, 59.131)), movement_box_size);

    auto current_movement_box_size = mm2px(Vec(15.736, 4.312));
    _current_movement_box = new AionTextBox(module, colors::BOX_BG_DARK, colors::OBSERVED_SUN, mm2px(Vec(3.604, 67.484)), current_movement_box_size);

    _prev_movement_box = new AionTextBox(module, colors::BOX_BG_DARK, colors::OBSERVED_SUN, mm2px(Vec(6.020, 75.551)), movement_box_size);
    _prev_prev_movement_box = new AionTextBox(module, colors::BOX_BG_DARK, colors::OBSERVED_SUN, mm2px(Vec(9.403, 83.562)), movement_box_size);

    addChild(_aion_box);
    addChild(_song_box);
    addChild(_next_next_movement_box);
    addChild(_next_movement_box);
    addChild(_current_movement_box);
    addChild(_prev_movement_box);
    addChild(_prev_prev_movement_box);
	}
};

} // namespace kokopellivcv
