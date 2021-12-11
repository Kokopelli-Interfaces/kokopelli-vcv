/** TODO

#pragma once

#include "Seasons.hpp"
#include "util/colors.hpp"

namespace kokopellivcv {

struct SeasonsValueDisplay : TextBox {
	Seasons *_module;
	int _previous_displayed_value = -1;

	SeasonsValueDisplay(TimeWheel *m) : TextBox() {
    _module = m;
    font = APP->window->loadFont(asset::plugin(pluginInstance, "/res/fonts/Nunito-Bold.ttf"));
    font_size = 12;
    letter_spacing = 0.f;
    backgroundColor = colors::BOX_BG;
    textColor = colors::ESTABLISHED;
    box.size = mm2px(Vec(6.902, 3.283));
    textOffset = Vec(box.size.x * 0.5f, 0.f);
    textAlign = NVG_ALIGN_CENTER | NVG_ALIGN_TOP;
  }

  void step() override {
    TextBox::step();
		if(_module) {
      kokopellivcv::dsp::circle::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }
      update(e);
    }
  }

  virtual void update(kokopellivcv::dsp::circle::Engine* engine) {
    return;
  };

	void setDisplayValue(int v) {
		std::string s;
		if(v != _previous_displayed_value) {
			_previous_displayed_value = v;
      s = string::f("%d", v);
      setText(s);
		}
	}
};

struct SongDisplay : SeasonsValueDisplay {
  using SeasonsValueDisplay::TimeWheelValueDisplay;
  SongDisplay(Seasons *m) : TimeWheelValueDisplay(m) {
    // FIXME
    // textOffset = Vec(box.size.x * 0.5f, 0.f);
  }

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    std::string s = e->_song.name;
    SeasonsValueDisplay::setText(s);
  }
};

struct PrevMovementDisplay : SeasonsValueDisplay {
  using SeasonsValueDisplay::TimeWheelValueDisplay;
  PrevMovementDisplay(Seasons *m) : TimeWheelValueDisplay(m) {
    // textOffset = Vec(box.size.x * 0.4f, box.size.y * 0.70f);
  }

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    std::string s;
    if (e->_song.current_movement->prev) {
      s = string::f("%c%d", e->_song.current_movement->prev->group, e->_song.current_movement->prev->group_movement_n);
    } else {
      s = "--";
    }
    SeasonsValueDisplay::setText(s);
  }
};


struct CurrentMovementDisplay : SeasonsValueDisplay {
  using SeasonsValueDisplay::TimeWheelValueDisplay;
  CurrentMovementDisplay(Seasons *m) : TimeWheelValueDisplay(m) {
    // textOffset = Vec(box.size.x * 0.4f, box.size.y * 0.70f);
  }

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    std::string s = string::f("%c%d", e->_song.current_movement->group, e->_song.current_movement->group_movement_n);
    SeasonsValueDisplay::setText(s);
  }
};

struct NextMovementDisplay : SeasonsValueDisplay {
  using SeasonsValueDisplay::TimeWheelValueDisplay;
  NextMovementDisplay(Seasons *m) : TimeWheelValueDisplay(m) {
    // textOffset = Vec(box.size.x * 0.4f, box.size.y * 0.70f);
  }

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    std::string s;
    if (e->_song.current_movement->next) {
      s = string::f("%c%d", e->_song.current_movement->next->group, e->_song.current_movement->next->group_movement_n);
    } else {
      s = "--";
    }
    SeasonsValueDisplay::setText(s);
  }
};

struct SeasonsWidget : ModuleWidget {
  const int hp = 6;

  bool _use_antipop = false;

  SongDisplay *song_display;

  PrevMovementDisplay *prev_movement_display;
  CurrentMovementDisplay *current_movement_display;
  NextMovementDisplay *next_movement_display;

  SeasonsWidget(TimeWheel *module) {
    setModule(module);
    box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Seasons.svg")));

		addChild(createWidget<KokopelliScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<KokopelliScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<KokopelliScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<KokopelliScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<MediumLEDButton>(mm2px(Vec(14.746, 54.259)), module, Seasons::TUNE_PARAM));
		addParam(createParam<LoveKnob>(mm2px(Vec(10.415, 72.580)), module, Seasons::LOVE_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(3.177, 79.26)), module, Seasons::CYCLE_DIVINITY_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(26.236, 79.26)), module, Seasons::CYCLE_FORWARD_PARAM));

		addInput(createInput<WombPort>(mm2px(Vec(24.280, 94.323)), module, Seasons::WOMB_INPUT));
		addInput(createInput<KokopelliPort>(mm2px(Vec(2.986, 107.705)), module, Seasons::PHASE_INPUT));
		addInput(createInput<KokopelliPort>(mm2px(Vec(13.475, 109.166)), module, Seasons::FOCUS_MODULATION_INPUT));

		addOutput(createOutput<KokopelliPort>(mm2px(Vec(2.986, 94.323)), module, Seasons::ESTABLISHED_OUTPUT));
		addOutput(createOutput<KokopelliPort>(mm2px(Vec(13.65, 95.875)), module, Seasons::SUN));

		addOutput(createOutput<KokopelliPort>(mm2px(Vec(24.216, 107.705)), module, Seasons::ESTABLISHED_PHASE_OUTPUT));

		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(16.214, 55.665)), module, Seasons::TUNE_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(4.641, 80.724)), module, Seasons::CYCLE_DIVINITY_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(27.701, 80.724)), module, Seasons::CYCLE_FORWARD_LIGHT));

    // FIXME have ONE display pLEASe

    // FIXME
    // // NOTE do the displays too
    // auto song_display_size = mm2px(Vec(28.126, 4.327));
		// song_display = new SongDisplay(module);
    // song_display->box.pos = mm2px(Vec(4.229, 36.138));
    // song_display->box.size = song_display_size;
    // song_display->textOffset = Vec(song_display->box.size.x * 0.5f, 0.f);
    // addChild(song_display);

    // auto group_display_size = mm2px(Vec(8.603, 4.327));

		// prev_movement_display = new PrevMovementDisplay(module);
    // prev_movement_display->box.pos = mm2px(Vec(1.649, 43.453));
    // prev_movement_display->box.size = group_display_size;
    // addChild(prev_movement_display);

		// current_movement_display = new CurrentMovementDisplay(module);
    // current_movement_display->box.pos = mm2px(Vec(13.793, 41.924));
    // current_movement_display->box.size = group_display_size;
    // addChild(current_movement_display);

		// next_movement_display = new NextMovementDisplay(module);
    // next_movement_display->box.pos = mm2px(Vec(25.926, 43.453));
    // next_movement_display->box.size = group_display_size;
    // addChild(next_movement_display);

  }

	void appendContextMenu(rack::Menu* menu) override {
		auto m = dynamic_cast<Seasons*>(module);
		assert(m);

    menu->addChild(new MenuLabel());
  }
};

} // namespace kokopellivcv

 */
