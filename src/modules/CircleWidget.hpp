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

struct EstablishedDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
  EstablishedDisplay(Circle *m) : CircleValueDisplay(m) {
    textOffset = Vec(box.size.x * 0.5f, box.size.y * 0.70f);
  }

  // race condition
  void update(kokopellivcv::dsp::circle::Engine* e) override {
    if (e->_gko.observer.checkIfInSubgroupMode()) {
      // textColor = colors::LOOK_BACK_LAYER;
      textColor = colors::ESTABLISHED;
      int pivot_id_len = e->_gko.observer._pivot_parent->id.size();
      std::string top_text = e->_gko.observer._pivot_parent->id.substr(0, pivot_id_len);

      std::string bottom_text;
      // this is an invariant that does not hold sometimes due to race condition as this is widget thread
      if ((unsigned int)pivot_id_len < e->_song.established->id.size()) {
        bottom_text = e->_song.established->id.substr(pivot_id_len+1);
      } else {
        bottom_text = e->_song.established->id.substr(pivot_id_len);
      }

      std::string s = string::f("%s|%s", top_text.c_str(), bottom_text.c_str());
      CircleValueDisplay::setText(s);
      // CircleValueDisplay::setText(e->_song.established->id);
    } else {
      textColor = colors::ESTABLISHED;
      // std::string s = string::f("%s", e->_song.established->id);
      // CircleValueDisplay::setText(s);
      CircleValueDisplay::setText(e->_song.established->id);
    }
  }
};

struct EstablishedBeatDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    int beat_in_established = e->_song.established->convertToBeat(e->_song.playhead, true);
    CircleValueDisplay::setDisplayValue(beat_in_established + 1);
	}
};

struct TotalEstablishedBeatDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;

  // FIXME
  void update(kokopellivcv::dsp::circle::Engine* e) override {
    CircleValueDisplay::setDisplayValue(e->_song.established->getTotalBeats());
	}
};

struct WombDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;

	WombDisplay(Circle *m) : CircleValueDisplay(m) {
    textOffset = Vec(box.size.x * 0.4f, box.size.y * 0.7f);
    textColor = colors::WOMB;
  }

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    int womb_display = e->_song.established->cycles_in_group.size() + 1;
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
    int beat_in_established = e->_song.established->convertToBeat(e->_song.new_cycle->playhead, false);
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

		addOutput(createOutput<EstablishedPort>(mm2px(Vec(2.276, 16.435)), module, Circle::ESTABLISHED_OUTPUT));
		addOutput(createOutput<SunPort>(mm2px(Vec(13.65, 15.365)), module, Circle::SUN));
		addInput(createInput<WombPort>(mm2px(Vec(24.925, 16.435)), module, Circle::WOMB_INPUT));

		addInput(createInput<PhasePort>(mm2px(Vec(2.276, 29.961)), module, Circle::PHASE_INPUT));
		addParam(createParam<AuditionKnob>(mm2px(Vec(12.699, 28.558)), module, Circle::AUDITION_PARAM));
		addOutput(createOutput<PhasePort>(mm2px(Vec(24.925, 29.961)), module, Circle::PHASE_OUTPUT));

		addParam(createParam<LoveKnob>(mm2px(Vec(10.415, 72.903)), module, Circle::LOVE_PARAM));

		addParam(createParam<MediumLEDButton>(mm2px(Vec(3.127, 80.017)), module, Circle::DIVINITY_PARAM));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(4.592, 81.481)), module, Circle::DIVINITY_LIGHT));

		addParam(createParam<MediumLEDButton>(mm2px(Vec(26.850, 80.017)), module, Circle::CYCLE_PARAM));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(28.315, 81.481)), module, Circle::CYCLE_LIGHT));

    auto focused_display_size = mm2px(Vec(8.48, 9.571));

		established_display = new EstablishedDisplay(module);
    established_display->box.pos = mm2px(Vec(1.388, 40.865));
    established_display->box.size = focused_display_size;
    addChild(established_display);

		womb_display = new WombDisplay(module);
    womb_display->box.pos = mm2px(Vec(25.828, 40.865));
    womb_display->box.size = focused_display_size;
    addChild(womb_display);

    auto beat_display_size = mm2px(Vec(6.385, 4.327));

    group_beat_display = new EstablishedBeatDisplay(module);
    group_beat_display->box.pos = mm2px(Vec(10.298, 40.865));
    group_beat_display->box.size = beat_display_size;
    addChild(group_beat_display);

    total_established_beats_display = new TotalEstablishedBeatDisplay(module);
    total_established_beats_display->box.pos = mm2px(Vec(10.298, 45.968));
    total_established_beats_display->box.size = beat_display_size;
    addChild(total_established_beats_display);


    womb_beat_display = new WombBeatDisplay(module);
    womb_beat_display->box.pos = mm2px(Vec(18.881, 40.865));
    womb_beat_display->box.size = beat_display_size;
    addChild(womb_beat_display);

    total_womb_beats_display = new TotalWombBeatDisplay(module);
    total_womb_beats_display->box.pos = mm2px(Vec(18.881, 45.968));
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
