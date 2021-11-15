#pragma once

#include "Circle.hpp"
#include "util/colors.hpp"

namespace kokopellivcv {

struct CircleValueDisplay : TextBox {
	Circle *_module;
	int _previous_displayed_value = -1;

	CircleValueDisplay(Circle *m) : TextBox() {
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

struct SongDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
  SongDisplay(Circle *m) : CircleValueDisplay(m) {
    // FIXME
    // textOffset = Vec(box.size.x * 0.5f, 0.f);
  }

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    std::string s = e->_song.name;
    CircleValueDisplay::setText(s);
  }
};

struct PrevMovementDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
  PrevMovementDisplay(Circle *m) : CircleValueDisplay(m) {
    // textOffset = Vec(box.size.x * 0.4f, box.size.y * 0.70f);
  }

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    std::string s;
    if (e->_song.current_movement->prev) {
      s = string::f("%c%d", e->_song.current_movement->prev->group, e->_song.current_movement->prev->group_movement_n);
    } else {
      s = "--";
    }
    CircleValueDisplay::setText(s);
  }
};


struct CurrentMovementDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
  CurrentMovementDisplay(Circle *m) : CircleValueDisplay(m) {
    // textOffset = Vec(box.size.x * 0.4f, box.size.y * 0.70f);
  }

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    std::string s = string::f("%c%d", e->_song.current_movement->group, e->_song.current_movement->group_movement_n);
    CircleValueDisplay::setText(s);
  }
};

struct NextMovementDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
  NextMovementDisplay(Circle *m) : CircleValueDisplay(m) {
    // textOffset = Vec(box.size.x * 0.4f, box.size.y * 0.70f);
  }

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    std::string s;
    if (e->_song.current_movement->next) {
      s = string::f("%c%d", e->_song.current_movement->next->group, e->_song.current_movement->next->group_movement_n);
    } else {
      s = "--";
    }
    CircleValueDisplay::setText(s);
  }
};

struct EstablishedDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
  EstablishedDisplay(Circle *m) : CircleValueDisplay(m) {
    textOffset = Vec(box.size.x * 0.4f, box.size.y * 0.70f);
  }

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    std::string s = string::f("%c", e->_song.current_movement->group);
    CircleValueDisplay::setText(s);
  }
};

struct EstablishedBeatDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    int beat_in_established = e->_song.established_group->convertToBeat(e->_song.playhead, true);
    CircleValueDisplay::setDisplayValue(beat_in_established + 1);
	}
};

struct TotalEstablishedBeatDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;

  // FIXME
  void update(kokopellivcv::dsp::circle::Engine* e) override {
    CircleValueDisplay::setDisplayValue(e->_song.established_group->getTotalBeats());
	}
};

struct WombDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;

	WombDisplay(Circle *m) : CircleValueDisplay(m) {
    textOffset = Vec(box.size.x * 0.4f, box.size.y * 0.7f);
    textColor = colors::WOMB;
  }

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    int womb_display = e->_song.cycles.size() + 1;
    std::string s = string::f("%d", womb_display);
    CircleValueDisplay::setText(s);
	}
};

struct WombBeatDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;

	WombBeatDisplay(Circle *m) : CircleValueDisplay(m) {
    textColor = colors::WOMB;
  }

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    if (e->_gko.tune_to_frequency_of_established) {
    int beat_in_established = e->_song.established_group->convertToBeat(e->_song.new_cycle->playhead, false);
      CircleValueDisplay::setDisplayValue(beat_in_established + 1);
    } else {
      CircleValueDisplay::setDisplayValue((int)e->_song.new_cycle->playhead);
    }
	}
};

struct TotalWombBeatDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;

	TotalWombBeatDisplay(Circle *m) : CircleValueDisplay(m) {
    textColor = colors::WOMB;
  }

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    if (e->_song.cycles.size() == 0) {
      CircleValueDisplay::setText("--");
      CircleValueDisplay::_previous_displayed_value = -1;
      return;
    }

    textColor = colors::LOOK_BACK_LAYER;
    CircleValueDisplay::setDisplayValue(e->getMostRecentCycleLength());
	}
};

struct CircleWidget : ModuleWidget {
  const int hp = 6;

  bool _use_antipop = false;

  SongDisplay *song_display;

  PrevMovementDisplay *prev_movement_display;
  CurrentMovementDisplay *current_movement_display;
  NextMovementDisplay *next_movement_display;

  EstablishedDisplay *established_display;
  WombDisplay *womb_display;

  WombBeatDisplay *womb_beat_display;
  TotalWombBeatDisplay *total_womb_beats_display;

  EstablishedBeatDisplay *group_beat_display;
  TotalEstablishedBeatDisplay *total_established_beats_display;

  CircleWidget(Circle *module) {
    setModule(module);
    box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Circle.svg")));

		addChild(createWidget<KokopelliScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<KokopelliScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<KokopelliScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<KokopelliScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<MediumLEDButton>(mm2px(Vec(14.746, 54.259)), module, Circle::TUNE_PARAM));
		addParam(createParam<LoveKnob>(mm2px(Vec(10.415, 72.580)), module, Circle::LOVE_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(3.177, 79.26)), module, Circle::ASCEND_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(26.236, 79.26)), module, Circle::CYCLE_FORWARD_PARAM));

		addInput(createInput<WombPort>(mm2px(Vec(24.280, 94.323)), module, Circle::WOMB_INPUT));
		addInput(createInput<KokopelliPort>(mm2px(Vec(2.986, 107.705)), module, Circle::PHASE_INPUT));
		addInput(createInput<KokopelliPort>(mm2px(Vec(13.475, 109.166)), module, Circle::FOCUS_MODULATION_INPUT));

		addOutput(createOutput<KokopelliPort>(mm2px(Vec(2.986, 94.323)), module, Circle::ESTABLISHED_OUTPUT));
		addOutput(createOutput<KokopelliPort>(mm2px(Vec(13.65, 95.875)), module, Circle::SUN));

		addOutput(createOutput<KokopelliPort>(mm2px(Vec(24.216, 107.705)), module, Circle::PHASE_OUTPUT));

		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(16.214, 55.665)), module, Circle::TUNE_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(4.641, 80.724)), module, Circle::ASCEND_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(27.701, 80.724)), module, Circle::CYCLE_FORWARD_LIGHT));

    // FIXME have ONE display pLEASe

    // NOTE do the displays too
    auto song_display_size = mm2px(Vec(28.126, 4.327));
		song_display = new SongDisplay(module);
    song_display->box.pos = mm2px(Vec(4.229, 36.138));
    song_display->box.size = song_display_size;
    song_display->textOffset = Vec(song_display->box.size.x * 0.5f, 0.f);
    addChild(song_display);

    auto group_display_size = mm2px(Vec(8.603, 4.327));

		prev_movement_display = new PrevMovementDisplay(module);
    prev_movement_display->box.pos = mm2px(Vec(1.649, 43.453));
    prev_movement_display->box.size = group_display_size;
    addChild(prev_movement_display);

		current_movement_display = new CurrentMovementDisplay(module);
    current_movement_display->box.pos = mm2px(Vec(13.793, 41.924));
    current_movement_display->box.size = group_display_size;
    addChild(current_movement_display);

		next_movement_display = new NextMovementDisplay(module);
    next_movement_display->box.pos = mm2px(Vec(25.926, 43.453));
    next_movement_display->box.size = group_display_size;
    addChild(next_movement_display);

    auto focused_display_size = mm2px(Vec(5.834, 9.571));

		established_display = new EstablishedDisplay(module);
    established_display->box.pos = mm2px(Vec(1.388, 51.671));
    established_display->box.size = focused_display_size;
    addChild(established_display);

		womb_display = new WombDisplay(module);
    womb_display->box.pos = mm2px(Vec(28.412, 51.671));
    womb_display->box.size = focused_display_size;
    addChild(womb_display);

    auto beat_display_size = mm2px(Vec(6.385, 4.327));

    group_beat_display = new EstablishedBeatDisplay(module);
    group_beat_display->box.pos = mm2px(Vec(7.75, 51.671));
    group_beat_display->box.size = beat_display_size;
    addChild(group_beat_display);

    womb_beat_display = new WombBeatDisplay(module);
    womb_beat_display->box.pos = mm2px(Vec(21.549, 51.671));
    womb_beat_display->box.size = beat_display_size;
    addChild(womb_beat_display);

    total_established_beats_display = new TotalEstablishedBeatDisplay(module);
    total_established_beats_display->box.pos = mm2px(Vec(7.75, 56.773));
    total_established_beats_display->box.size = beat_display_size;
    addChild(total_established_beats_display);

    total_womb_beats_display = new TotalWombBeatDisplay(module);
    total_womb_beats_display->box.pos = mm2px(Vec(21.549, 56.773));
    total_womb_beats_display->box.size = beat_display_size;
    addChild(total_womb_beats_display);
  }

	void appendContextMenu(rack::Menu* menu) override {
		auto m = dynamic_cast<Circle*>(module);
		assert(m);

    menu->addChild(new MenuLabel());

    // menu->addChild(new BoolOptionMenuItem("Use read antipop at phase discontuinity", [m]() {
    //   return &m->_options.use_antipop;
    // }));

    // menu->addChild(new BoolOptionMenuItem("Bipolar Phase Input (-5V to 5V)", [m]() {
    //   return &m->_options.bipolar_phase_input;
    // }));

		FadeSliderItem *love_resolution_slider = new FadeSliderItem(&m->_options.love_resolution, "Love Resolution");
		love_resolution_slider->box.size.x = 190.f;
		menu->addChild(love_resolution_slider);

    // TODO
    // menu->addChild(new Slider());
  }
};

} // namespace kokopellivcv
