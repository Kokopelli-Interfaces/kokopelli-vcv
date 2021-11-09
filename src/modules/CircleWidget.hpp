#pragma once


#include "Circle.hpp"
#include "util/colors.hpp"

namespace kokopellivcv {

struct CircleValueDisplay : TextBox {
	Circle *_module;
	int _backward_displayed_value = -1;

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

	void setDisplayValue(int v) {
		std::string s;
		if(v != _backward_displayed_value) {
			_backward_displayed_value = v;
      s = string::f("%d", v);
      setText(s);
		}
	}
};

struct SongDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
  SongDisplay(Circle *m) : CircleValueDisplay(m) {
    // FIXME
    // textOffset = Vec(box.size.x * 2.5f, 0.f);
  }

  void step() override {
    CircleValueDisplay::step();
		if(_module) {
      kokopellivcv::dsp::circle::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }

      std::string s = e->song.name;
      CircleValueDisplay::setText(s);
    }
  }
};



struct PrevSectionDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
  PrevSectionDisplay(Circle *m) : CircleValueDisplay(m) {
    // textOffset = Vec(box.size.x * 0.4f, box.size.y * 0.70f);
  }

  void step() override {
    CircleValueDisplay::step();
		if(_module) {
      kokopellivcv::dsp::circle::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }

      std::string s;
      if (e->_current_section->prev) {
        s = string::f("%c%d", e->_current_section->prev->group, e->_current_section->prev->group_section_n);
      } else {
        s = "--";
      }
      CircleValueDisplay::setText(s);
    }
  }
};


struct CurrentSectionDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
  CurrentSectionDisplay(Circle *m) : CircleValueDisplay(m) {
    // textOffset = Vec(box.size.x * 0.4f, box.size.y * 0.70f);
  }

  void step() override {
    CircleValueDisplay::step();
		if(_module) {
      kokopellivcv::dsp::circle::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }

      std::string s = string::f("%c%d", e->_current_section->group, e->_current_section->group_section_n);
      CircleValueDisplay::setText(s);
    }
  }
};

struct NextSectionDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
  NextSectionDisplay(Circle *m) : CircleValueDisplay(m) {
    // textOffset = Vec(box.size.x * 0.4f, box.size.y * 0.70f);
  }

  void step() override {
    CircleValueDisplay::step();
		if(_module) {
      kokopellivcv::dsp::circle::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }

      std::string s;
      if (e->_current_section->next) {
        s = string::f("%c%d", e->_current_section->next->group, e->_current_section->next->group_section_n);
      } else {
        s = "--";
      }
      CircleValueDisplay::setText(s);
    }
  }
};

struct EstablishedDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
  EstablishedDisplay(Circle *m) : CircleValueDisplay(m) {
    textOffset = Vec(box.size.x * 0.4f, box.size.y * 0.70f);
  }

  void step() override {
    CircleValueDisplay::step();
		if(_module) {
      kokopellivcv::dsp::circle::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }

      std::string s = string::f("%c", e->_current_section->group);
      CircleValueDisplay::setText(s);
    }
  }
};

struct EstablishedBeatDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
	void step() override {
		CircleValueDisplay::step();
		if(_module) {
      kokopellivcv::dsp::circle::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }

      int section_beat_display = e->song._position.beat - e->_current_section->start_beat + 1;

      CircleValueDisplay::setDisplayValue(section_beat_display);
		}
	}
};

struct TotalEstablishedBeatDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
	void step() override {
		CircleValueDisplay::step();
		if(_module) {
      kokopellivcv::dsp::circle::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }
      unsigned int n_circle_beats = e->_current_section->end_beat - e->_current_section->start_beat;
      CircleValueDisplay::setDisplayValue(n_circle_beats);
		}
	}
};

struct WombDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;

	WombDisplay(Circle *m) : CircleValueDisplay(m) {
    textOffset = Vec(box.size.x * 0.4f, box.size.y * 0.7f);
  }

	void step() override {
		CircleValueDisplay::step();
		if(_module) {
      kokopellivcv::dsp::circle::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }

      textColor = defaultTextColor;
      int womb_display = 0;
      // bool show_new_cycle = true;
      // bool show_new_cycle = !(e->_love_direction == LoveDirection::ESTABLISHED && e->_tune_to_frequency_of_established);
      // if (show_new_cycle) {
        textColor = colors::WOMB;
        // womb_display = e->song.getNumberOfActiveCycles() + 1;
        womb_display = e->song.cycles.size() + 1;
      // } else {
      //   womb_display = e->_focused_cycle_i + 1;
        // womb_display = e->song.getNumberOfActiveCycles();
        // if (e->song.cycles.size() == 0) {
        //   CircleValueDisplay::setText("--");
        //   CircleValueDisplay::_backward_displayed_value = -1;
        //   return;
        // }
      // }

      std::string s = string::f("%d", womb_display);
      CircleValueDisplay::setText(s);
		}
	}
};

struct WombBeatDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
	void step() override {
		CircleValueDisplay::step();
		if(_module) {
      kokopellivcv::dsp::circle::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }

      int womb_beat_display = 0;

      textColor = colors::WOMB;
      // bool show_new_cycle = true;
      // bool show_new_cycle = !(e->_love_direction == LoveDirection::ESTABLISHED && e->_tune_to_frequency_of_established);
      // if (e->isRecording() && show_new_cycle) {
      // textColor = nvgRGB(0x9b, 0x44, 0x42); // red

      womb_beat_display = e->_new_cycle->_write_beat;

      // } else if (e->song.cycles.size() != 0) {
      //   // TODO how to show start position of loops?
      //   womb_beat_display = e->song.cycles[e->_focused_cycle_i]->getCycleBeat(e->song._position.beat);
      // } else {
      //   CircleValueDisplay::setText("--");
      //   CircleValueDisplay::_backward_displayed_value = -1;
      //   return;
      // }

      CircleValueDisplay::setDisplayValue(womb_beat_display + 1);
		}
	}
};

struct TotalWombBeatDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
	void step() override {
		CircleValueDisplay::step();
		if(_module) {
      kokopellivcv::dsp::circle::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }

      int total_cycle_beats = 0;
      textColor = colors::WOMB;
      // bool show_new_cycle = true;
      // bool show_new_cycle = !(e->_love_direction == LoveDirection::ESTABLISHED && e->_tune_to_frequency_of_established);
      // if (show_new_cycle) {
        // if (e->_tune_to_frequency_of_established) {
        //   total_cycle_beats = e->_new_cycle->_n_beats;
        if (e->song.cycles.size() != 0) {
          total_cycle_beats = e->getMostRecentLoopLength();
          textColor = colors::LOOK_BACK_LAYER;
        } else {
          CircleValueDisplay::setText("--");
          CircleValueDisplay::_backward_displayed_value = -1;
          return;
        }
      // } else if (e->song.cycles.size() != 0) {
      //   total_cycle_beats = e->song.cycles[e->_focused_cycle_i]->_n_beats;
      // } else {
      //   CircleValueDisplay::setText("--");
      //   CircleValueDisplay::_backward_displayed_value = -1;
      //   return;
      // }

      CircleValueDisplay::setDisplayValue(total_cycle_beats);
		}
	}
};


struct CircleWidget : ModuleWidget {
  const int hp = 6;

  bool _use_antipop = false;

  SongDisplay *song_display;

  PrevSectionDisplay *prev_section_display;
  CurrentSectionDisplay *current_section_display;
  NextSectionDisplay *next_section_display;

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
		addParam(createParam<MediumLEDButton>(mm2px(Vec(3.177, 79.26)), module, Circle::BACKWARD_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(26.236, 79.26)), module, Circle::FORWARD_PARAM));

		addInput(createInput<WombPort>(mm2px(Vec(24.280, 94.323)), module, Circle::WOMB_INPUT));
		addInput(createInput<KokopelliPort>(mm2px(Vec(2.986, 107.705)), module, Circle::PHASE_INPUT));
		addInput(createInput<KokopelliPort>(mm2px(Vec(13.475, 109.166)), module, Circle::FOCUS_MODULATION_INPUT));

		addOutput(createOutput<KokopelliPort>(mm2px(Vec(2.986, 94.323)), module, Circle::ESTABLISHED_OUTPUT));
		addOutput(createOutput<KokopelliPort>(mm2px(Vec(13.65, 95.875)), module, Circle::SUN));

		addOutput(createOutput<KokopelliPort>(mm2px(Vec(24.216, 107.705)), module, Circle::PHASE_OUTPUT));

		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(16.214, 55.665)), module, Circle::TUNE_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(4.641, 80.724)), module, Circle::BACKWARD_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(27.701, 80.724)), module, Circle::FORWARD_LIGHT));

    // FIXME have ONE display pLEASe

    // NOTE do the displays too
    auto song_display_size = mm2px(Vec(28.126, 4.327));
		song_display = new SongDisplay(module);
    song_display->box.pos = mm2px(Vec(4.229, 36.138));
    song_display->box.size = song_display_size;
    addChild(song_display);

    auto group_display_size = mm2px(Vec(8.603, 4.327));

		prev_section_display = new PrevSectionDisplay(module);
    prev_section_display->box.pos = mm2px(Vec(1.649, 43.453));
    prev_section_display->box.size = group_display_size;
    addChild(prev_section_display);

		current_section_display = new CurrentSectionDisplay(module);
    current_section_display->box.pos = mm2px(Vec(13.793, 41.924));
    current_section_display->box.size = group_display_size;
    addChild(current_section_display);

		next_section_display = new NextSectionDisplay(module);
    next_section_display->box.pos = mm2px(Vec(25.926, 43.453));
    next_section_display->box.size = group_display_size;
    addChild(next_section_display);


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

    menu->addChild(new BoolOptionMenuItem("Use read antipop at phase discontuinity", [m]() {
      return &m->_options.use_antipop;
    }));

    menu->addChild(new BoolOptionMenuItem("Bipolar Phase Input (-5V to 5V)", [m]() {
      return &m->_options.bipolar_phase_input;
    }));

    menu->addChild(new BoolOptionMenuItem("Bipolar Phase Input (-5V to 5V)", [m]() {
      return &m->_options.bipolar_phase_input;
    }));

		FadeSliderItem *love_resolution_slider = new FadeSliderItem(&m->_options.love_resolution, "Love Resolution");
		love_resolution_slider->box.size.x = 190.f;
		menu->addChild(love_resolution_slider);

    // TODO
    // menu->addChild(new Slider());
  }
};

} // namespace kokopellivcv
