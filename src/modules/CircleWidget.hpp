#pragma once

#include "Circle.hpp"
#include "util/colors.hpp"

namespace kokopellivcv {

struct CircleTextBox : TextBox {
	Circle *_module;
	int _previous_displayed_value = -1;

	CircleTextBox(Circle *m, NVGcolor background_color, NVGcolor text_color, rack::math::Vec pos, rack::math::Vec size) : TextBox() {
    _module = m;
    font = APP->window->loadFont(asset::plugin(pluginInstance, "/res/fonts/Nunito-Bold.ttf"));
    font_size = 12;
    letter_spacing = 0.f;
    backgroundColor = background_color;
    textColor = text_color;
    box.pos = pos;
    box.size = size;
    textOffset = Vec(box.size.x * 0.5f, 0.f);
    textAlign = NVG_ALIGN_CENTER | NVG_ALIGN_TOP;
  }

  void step() override {
    TextBox::step();
		if(_module) {
      kokopellivcv::dsp::circle::Engine* e = _module->_engines[0];
      if (!e) {
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

struct EstablishedDisplay : CircleTextBox {
  using CircleTextBox::CircleTextBox;

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    if (e) {
      float phase = e->_song.established->getPhase(e->_song.playhead);
      backgroundColor.a = phase;
    }

    if (e->_gko.observer.checkIfInSubgroupMode()) {
      // textColor = colors::LOOK_BACK_LAYER;
      textColor = colors::ESTABLISHED;
      int pivot_id_len = e->_gko.observer._pivot_parent->id.size();
      std::string left_text = e->_gko.observer._pivot_parent->id.substr(0, pivot_id_len);

      std::string right_text;
      // this is an invariant that does not hold sometimes due to race condition as this is widget thread
      if ((unsigned int)pivot_id_len < e->_song.established->id.size()) {
        right_text = e->_song.established->id.substr(pivot_id_len+1);
      } else {
        right_text = e->_song.established->id.substr(pivot_id_len);
      }

      std::string s = string::f("%s | %s", left_text.c_str(), right_text.c_str());
      CircleTextBox::setText(s);
      // CircleTextBox::setText(e->_song.established->id);
    } else {
      textColor = colors::ESTABLISHED;
      // std::string s = string::f("%s", e->_song.established->id);
      // CircleTextBox::setText(s);
      CircleTextBox::setText(e->_song.established->id);
    }
  }
};

struct EstablishedBeatDisplay : CircleTextBox {
  using CircleTextBox::CircleTextBox;

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    int beat_in_established = e->_song.established->convertToBeat(e->_song.playhead, true);
    CircleTextBox::setDisplayValue(beat_in_established + 1);
	}
};

struct TotalEstablishedBeatDisplay : CircleTextBox {
  using CircleTextBox::CircleTextBox;

  // FIXME
  void update(kokopellivcv::dsp::circle::Engine* e) override {
    CircleTextBox::setDisplayValue(e->_song.established->getTotalBeats());
	}
};

struct WombDisplay : CircleTextBox {
  using CircleTextBox::CircleTextBox;

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    float phase = 0.f;
    if (e->_gko.observer.checkIfInSubgroupMode()) {
      phase = e->_song.established->getBeatPhase(e->_song.playhead);
    }

    backgroundColor.a = phase;

    int womb_display = e->_song.established->cycles_in_group.size() + 1;
    std::string s = string::f("%d", womb_display);
    CircleTextBox::setText(s);
	}
};

struct WombBeatDisplay : CircleTextBox {
  using CircleTextBox::CircleTextBox;

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    if (e->_gko.tune_to_frequency_of_established) {
      int beat_in_established;
      if (e->_gko.observer.checkIfInSubgroupMode()) {
        beat_in_established = e->_song.established->convertToBeat(e->_song.new_cycle->playhead, true);
      } else {
        beat_in_established = e->_song.established->convertToBeat(e->_song.new_cycle->playhead, false);
      }

      CircleTextBox::setDisplayValue(beat_in_established + 1);
    } else {
      CircleTextBox::setDisplayValue((int)e->_song.new_cycle->playhead);
    }
	}
};

struct TotalWombBeatDisplay : CircleTextBox {
  using CircleTextBox::CircleTextBox;

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    if (e->_song.cycles.size() == 0) {
      textColor = colors::WOMB;
      CircleTextBox::setText("--");
      CircleTextBox::_previous_displayed_value = -1;
      return;
    }

    textColor = colors::LOOK_BACK_LAYER;
    if (e->_gko.observer.checkIfInSubgroupMode()) {
      CircleTextBox::setDisplayValue(e->_song.established->getTotalBeats());
    } else {
      CircleTextBox::setDisplayValue(e->getMostRecentCycleLength());
    }
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

		addInput(createInput<PhaseInPort>(mm2px(Vec(24.925, 29.961)), module, Circle::PHASE_INPUT));
		addParam(createParam<AuditionKnob>(mm2px(Vec(12.699, 28.558)), module, Circle::AUDITION_PARAM));
		addOutput(createOutput<PhaseOutPort>(mm2px(Vec(2.276, 29.961)), module, Circle::PHASE_OUTPUT));

		addParam(createParam<LoveKnob>(mm2px(Vec(10.415, 72.903)), module, Circle::LOVE_PARAM));

		addParam(createParam<MediumLEDButton>(mm2px(Vec(3.127, 80.017)), module, Circle::DIVINITY_PARAM));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(4.592, 81.481)), module, Circle::DIVINITY_LIGHT));

		addParam(createParam<MediumLEDButton>(mm2px(Vec(26.850, 80.017)), module, Circle::CYCLE_PARAM));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(28.315, 81.481)), module, Circle::CYCLE_LIGHT));

    auto focused_display_size = mm2px(Vec(15.736, 4.312));
		established_display = new EstablishedDisplay(module, colors::BOX_BG_LIGHT, colors::ESTABLISHED, mm2px(Vec(1.236, 40.865)), focused_display_size);
		womb_display = new WombDisplay(module, colors::BOX_BG_LIGHT, colors::WOMB, mm2px(Vec(18.644, 40.865)), focused_display_size);

    auto beat_display_size = mm2px(Vec(7.527, 4.327));
    group_beat_display = new EstablishedBeatDisplay(module, colors::BOX_BG_DARK, colors::ESTABLISHED, mm2px(Vec(1.268, 45.968)), beat_display_size);
    total_established_beats_display = new TotalEstablishedBeatDisplay(module, colors::BOX_BG_DARK, colors::ESTABLISHED, mm2px(Vec(9.321, 45.968)), beat_display_size);
    womb_beat_display = new WombBeatDisplay(module, colors::BOX_BG_DARK, colors::WOMB, mm2px(Vec(18.675, 45.968)), beat_display_size);
    total_womb_beats_display = new TotalWombBeatDisplay(module, colors::BOX_BG_DARK, colors::WOMB, mm2px(Vec(26.729, 45.968)), beat_display_size);

    addChild(established_display);
    addChild(womb_display);
    addChild(group_beat_display);
    addChild(total_established_beats_display);
    addChild(womb_beat_display);
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
