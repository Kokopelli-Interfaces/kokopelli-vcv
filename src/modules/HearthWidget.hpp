#pragma once

#include "Hearth.hpp"
#include "util/colors.hpp"

namespace kokopellivcv {

struct HearthTextBox : TextBox {
	Hearth *_module = nullptr;
	int _previous_displayed_value = -1;

	HearthTextBox(Hearth *m, NVGcolor background_color, NVGcolor text_color, rack::math::Vec pos, rack::math::Vec size) : TextBox() {
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

  void step() override {
    TextBox::step();
		if(_module) {
      if (_module->channels() == 0) {
        return;
      }
      kokopellivcv::dsp::hearth::Engine* e = _module->_engines[0];
      update(e);
    }
  }

  virtual void update(kokopellivcv::dsp::hearth::Engine* engine) {
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

struct ObservedSunDisplay : HearthTextBox {
  using HearthTextBox::HearthTextBox;

  void update(kokopellivcv::dsp::hearth::Engine* e) override {
    // float phase = e->_village.observed_sun->getPhase(e->_village.playhead);
    float phase = e->_village.observed_sun->getPhase();
    backgroundColor.a = phase;

    if (e->_gko._observer.checkIfInSubgroupMode()) {
      // textColor = colors::LOOK_BACK_LAYER;
      textColor = colors::OBSERVED_SUN;
      int pivot_id_len = e->_gko._observer._pivot_parent->name.size();
      std::string left_text = e->_gko._observer._pivot_parent->name.substr(0, pivot_id_len);

      std::string right_text;
      // this is an invariant that does not hold sometimes due to race condition as this is widget thread
      if ((unsigned int)pivot_id_len < e->_village.observed_sun->name.size()) {
        right_text = e->_village.observed_sun->name.substr(pivot_id_len+1);
      } else {
        right_text = e->_village.observed_sun->name.substr(pivot_id_len);
      }

      std::string s = string::f("%s | %s", left_text.c_str(), right_text.c_str());
      HearthTextBox::setText(s);
      // HearthTextBox::setText(e->_village.observed_sun->name);
    } else {
      textColor = colors::OBSERVED_SUN;
      // std::string s = string::f("%s", e->_village.observed_sun->name);
      // HearthTextBox::setText(s);
      HearthTextBox::setText(e->_village.observed_sun->name);
    }
  }
};

struct ObservedSunBeatDisplay : HearthTextBox {
  using HearthTextBox::HearthTextBox;

  void update(kokopellivcv::dsp::hearth::Engine* e) override {
    int beat_in_observed_sun = e->_village.observed_sun->getBeatN();
    HearthTextBox::setDisplayValue(beat_in_observed_sun + 1);
	}
};

struct TotalObservedSunBeatDisplay : HearthTextBox {
  using HearthTextBox::HearthTextBox;

  // FIXME
  void update(kokopellivcv::dsp::hearth::Engine* e) override {
    HearthTextBox::setDisplayValue(e->_village.observed_sun->getTotalBeats());
	}
};

struct WombDisplay : HearthTextBox {
  using HearthTextBox::HearthTextBox;

  void update(kokopellivcv::dsp::hearth::Engine* e) override {
    float phase = 0.f;
    // phase = e->_village.observed_sun->getBeatPhase(e->_village.new_voice->playhead);
    phase = e->_village.observed_sun->getBeatPhase();

    backgroundColor.a = phase;

    int womb_display = e->_village.observed_sun->_voices_in_group.size() + 1;
    std::string s = string::f("%d", womb_display);
    HearthTextBox::setText(s);
	}
};

struct WombBeatDisplay : HearthTextBox {
  using HearthTextBox::HearthTextBox;

  void update(kokopellivcv::dsp::hearth::Engine* e) override {
    if (e->_gko.tune_to_frequency_of_observed_sun) {
      int beat_in_observed_sun;
      beat_in_observed_sun = e->_village.observed_sun->getBeatN();

      HearthTextBox::setDisplayValue(beat_in_observed_sun + 1);
    } else {
      HearthTextBox::setDisplayValue((int)e->_village.new_voice->playhead);
    }
	}
};

struct TotalWombBeatDisplay : HearthTextBox {
  using HearthTextBox::HearthTextBox;

  void update(kokopellivcv::dsp::hearth::Engine* e) override {
    if (e->_village.voices.size() == 0) {
      textColor = colors::WOMB;
      HearthTextBox::setText("--");
      HearthTextBox::_previous_displayed_value = -1;
      return;
    }

    // option
    if (e->_gko._observer.checkIfInSubgroupMode()) {
      HearthTextBox::setDisplayValue(e->_village.observed_sun->getTotalBeats());
    } else {
      textColor = colors::LOOK_BACK_LAYER;
      HearthTextBox::setDisplayValue(e->getMostRecentVoiceLength());
      // if (e->_village.observed_sun->_voices_in_group.size() == 0) {
      //   HearthTextBox::setDisplayValue(1);
      // } else {
      //   Time adjusted_period = e->_village.observed_sun->getAdjustedPeriod(e->_village.new_voice->playhead);
      //   int n_beats = e->_village.observed_sun->getBeatN();
      //   HearthTextBox::setDisplayValue(n_beats);
      // }
    }
	}
};

struct HearthWidget : ModuleWidget {
  const int hp = 6;

  ObservedSunDisplay *observed_sun_display;
  WombDisplay *womb_display;

  WombBeatDisplay *womb_beat_display;
  TotalWombBeatDisplay *total_womb_beats_display;

  ObservedSunBeatDisplay *group_beat_display;
  TotalObservedSunBeatDisplay *total_observed_sun_beats_display;

  HearthWidget(Hearth *module) {
    setModule(module);
    box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Hearth.svg")));

		addChild(createWidget<KokopelliScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<KokopelliScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<KokopelliScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<KokopelliScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addOutput(createOutput<ObservedSunPort>(mm2px(Vec(2.276, 16.435)), module, Hearth::OBSERVER_OUTPUT));
		addOutput(createOutput<SunPort>(mm2px(Vec(13.65, 15.365)), module, Hearth::SUN));
		addInput(createInput<WombPort>(mm2px(Vec(24.925, 16.435)), module, Hearth::WOMB_INPUT));

		addOutput(createOutput<PhaseOutPort>(mm2px(Vec(24.925, 29.961)), module, Hearth::OBSERVER_BEAT_PHASE_OUTPUT));
		addParam(createParam<AuditionKnob>(mm2px(Vec(12.699, 28.719)), module, Hearth::AUDITION_PARAM));
		addOutput(createOutput<PhaseOutPort>(mm2px(Vec(2.276, 29.961)), module, Hearth::OBSERVER_PHASE_OUTPUT));

		addParam(createParam<LoveKnob>(mm2px(Vec(10.220, 76.609)), module, Hearth::LOVE_PARAM));

		addParam(createParam<MediumLEDButton>(mm2px(Vec(3.127, 85.334)), module, Hearth::DIVINITY_PARAM));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(4.592, 86.798)), module, Hearth::DIVINITY_LIGHT));

		addParam(createParam<MediumLEDButton>(mm2px(Vec(26.850, 85.333)), module, Hearth::NEXT_MOVEMENT_PARAM));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(28.315, 86.798)), module, Hearth::NEXT_MOVEMENT_LIGHT));

    auto focused_display_size = mm2px(Vec(15.736, 4.312));
		observed_sun_display = new ObservedSunDisplay(module, colors::BOX_BG_LIGHT, colors::OBSERVED_SUN, mm2px(Vec(1.236, 40.865)), focused_display_size);
		womb_display = new WombDisplay(module, colors::BOX_BG_LIGHT, colors::WOMB, mm2px(Vec(18.644, 40.865)), focused_display_size);

    auto beat_display_size = mm2px(Vec(7.527, 4.327));
    group_beat_display = new ObservedSunBeatDisplay(module, colors::BOX_BG_DARK, colors::OBSERVED_SUN, mm2px(Vec(1.268, 45.968)), beat_display_size);
    total_observed_sun_beats_display = new TotalObservedSunBeatDisplay(module, colors::BOX_BG_DARK, colors::OBSERVED_SUN, mm2px(Vec(9.321, 45.968)), beat_display_size);
    womb_beat_display = new WombBeatDisplay(module, colors::BOX_BG_DARK, colors::WOMB, mm2px(Vec(18.675, 45.968)), beat_display_size);
    total_womb_beats_display = new TotalWombBeatDisplay(module, colors::BOX_BG_DARK, colors::WOMB, mm2px(Vec(26.729, 45.968)), beat_display_size);

    addChild(observed_sun_display);
    addChild(womb_display);
    addChild(group_beat_display);
    addChild(total_observed_sun_beats_display);
    addChild(womb_beat_display);
    addChild(total_womb_beats_display);
  }

	void appendContextMenu(rack::Menu* menu) override {
		auto m = dynamic_cast<Hearth*>(module);
		assert(m);

    menu->addChild(new MenuLabel());

    // menu->addChild(new MenuLabel("Phase Output"));
    // menu->addChild(new SpacerOptionMenuItem());

    // OptionsMenuItem* phase_selection = new OptionsMenuItem("Output Phase Selection");
    // phase_selection->addItem(OptionMenuItem("ObservedSun Beat Period", [m]() { return m->_options.output_beat_phase; }, [m]() { m->_options.output_beat_phase  = true; }));
    // phase_selection->addItem(OptionMenuItem("ObservedSun Period", [m]() { return !m->_options.output_beat_phase; }, [m]() { m->_options.output_beat_phase = false; }));
    // OptionsMenuItem::addToMenu(phase_selection, menu);

    // menu->addChild(new MenuLabel("Love Resolution"));

    // menu->addChild(new SpacerOptionMenuItem());

		FadeSliderItem *love_resolution_slider = new FadeSliderItem(&m->_options.love_resolution, "Love Resolution");
		love_resolution_slider->box.size.x = 190.f;
		menu->addChild(love_resolution_slider);

    // FIXME
		// DelayShiftbackSlider *delay_shiftback_slider = new DelayShiftbackSlider(&m->_options.delay_shiftback, "Input Delay Shiftback");
		// delay_shiftback_slider->box.size.x = 190.f;
		// menu->addChild(delay_shiftback_slider);

    menu->addChild(new BoolOptionMenuItem("Include moon in sun output", [m]() {
      return &m->_options.include_moon_in_sun_output;
    }));

    menu->addChild(new BoolOptionMenuItem("Include unloved moon in sun output", [m]() {
      return &m->_options.include_unloved_moon_in_sun_output;
    }));

    // OptionsMenuItem::addToMenu(love_resolution, menu);

    // TODO
    // menu->addChild(new Slider());
  }
};

} // namespace kokopellivcv
