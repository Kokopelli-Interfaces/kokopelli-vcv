#pragma once

#include "Circle.hpp"
#include "util/colors.hpp"

namespace kokopellivcv {

struct CircleTextBox : TextBox {
	Circle *_module = nullptr;
	int _previous_displayed_value = -1;

	CircleTextBox(Circle *m, NVGcolor background_color, NVGcolor text_color, rack::math::Vec pos, rack::math::Vec size) : TextBox() {
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
		if(_module && _module->_initialized) {
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

struct ObservedSunDisplay : CircleTextBox {
  using CircleTextBox::CircleTextBox;

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    float phase = e->_song.observed_sun->getPhase(e->_song.playhead);
    if (e->options.poly_input_phase_mode) {
      backgroundColor = colors::CYAN;
    } else {
      backgroundColor = colors::BOX_BG_LIGHT;
    }

    backgroundColor.a = phase;

    if (e->_gko.observer.checkIfInSubgroupMode()) {
      // textColor = colors::LOOK_BACK_LAYER;
      textColor = colors::OBSERVED_SUN;
      int pivot_id_len = e->_gko.observer._pivot_parent->id.size();
      std::string left_text = e->_gko.observer._pivot_parent->id.substr(0, pivot_id_len);

      std::string right_text;
      // this is an invariant that does not hold sometimes due to race condition as this is widget thread
      if ((unsigned int)pivot_id_len < e->_song.observed_sun->id.size()) {
        right_text = e->_song.observed_sun->id.substr(pivot_id_len+1);
      } else {
        right_text = e->_song.observed_sun->id.substr(pivot_id_len);
      }

      std::string s = string::f("%s | %s", left_text.c_str(), right_text.c_str());
      CircleTextBox::setText(s);
      // CircleTextBox::setText(e->_song.observed_sun->id);
    } else {
      textColor = colors::OBSERVED_SUN;
      // std::string s = string::f("%s", e->_song.observed_sun->id);
      // CircleTextBox::setText(s);
      CircleTextBox::setText(e->_song.observed_sun->id);
    }
  }
};

struct ObservedSunBeatDisplay : CircleTextBox {
  using CircleTextBox::CircleTextBox;

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    int beat_in_observed_sun = e->_song.observed_sun->convertToBeat(e->_song.playhead, true);
    CircleTextBox::setDisplayValue(beat_in_observed_sun + 1);
	}
};

struct TotalObservedSunBeatDisplay : CircleTextBox {
  using CircleTextBox::CircleTextBox;

  // FIXME
  void update(kokopellivcv::dsp::circle::Engine* e) override {
    CircleTextBox::setDisplayValue(e->_song.observed_sun->getTotalBeats());
	}
};

struct BandDisplay : CircleTextBox {
  using CircleTextBox::CircleTextBox;

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    if (e->isRecording()) {
      textColor = colors::BAND;
    } else {
      textColor = colors::OBSERVED_SUN;
    }

    float phase = 0.f;
    // phase = e->_song.observed_sun->getBeatPhase(e->_song.new_cycle->playhead);
    phase = e->_song.observed_sun->getBeatPhase(e->_song.playhead);

    if (e->options.poly_input_phase_mode) {
      backgroundColor = colors::CYAN;
    } else {
      backgroundColor = colors::BOX_BG_LIGHT;
    }
    backgroundColor.a = phase;

    int band_display = e->_song.observed_sun->cycles_in_group.size() + 1;
    std::string s = string::f("%d", band_display);
    CircleTextBox::setText(s);
	}
};

struct BandBeatDisplay : CircleTextBox {
  using CircleTextBox::CircleTextBox;

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    int beat_in_observed_sun = 0;
    if (e->isRecording()) {
      textColor = colors::BAND;
      if (e->_song.cycles.empty() && e->_gko.use_ext_phase) {
        beat_in_observed_sun = (int) e->_song.new_cycle->playhead;
      } else {
        beat_in_observed_sun = e->_song.observed_sun->convertToBeat(e->_song.new_cycle->playhead, false);
      }
    }

    CircleTextBox::setDisplayValue(beat_in_observed_sun + 1);
	}
};

struct TotalBandBeatDisplay : CircleTextBox {
  using CircleTextBox::CircleTextBox;

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    if (e->_song.cycles.size() == 0) {
      textColor = colors::BAND;
      CircleTextBox::setText("--");
      CircleTextBox::_previous_displayed_value = -1;
      return;
    }

    if (e->isRecording()) {
      textColor = colors::BAND;
    } else {
      textColor = colors::OBSERVED_SUN;;
    }

    // option
    if (e->_gko.observer.checkIfInSubgroupMode()) {
      CircleTextBox::setDisplayValue(e->_song.observed_sun->getTotalBeats());
    } else {
      textColor = colors::LOOK_BACK_LAYER;
      CircleTextBox::setDisplayValue(e->getMostRecentCycleLength());
      // if (e->_song.observed_sun->cycles_in_group.size() == 0) {
      //   CircleTextBox::setDisplayValue(1);
      // } else {
      //   Time adjusted_period = e->_song.observed_sun->getAdjustedPeriod(e->_song.new_cycle->playhead);
      //   int n_beats = e->_song.observed_sun->convertToBeat(adjusted_period, false);
      //   CircleTextBox::setDisplayValue(n_beats);
      // }
    }
	}
};

struct CircleWidget : ModuleWidget {
  const int hp = 6;

  ObservedSunDisplay *observed_sun_display;
  BandDisplay *band_display;

  BandBeatDisplay *band_beat_display;
  TotalBandBeatDisplay *total_band_beats_display;

  ObservedSunBeatDisplay *group_beat_display;
  TotalObservedSunBeatDisplay *total_observed_sun_beats_display;

  CircleWidget(Circle *module) {
    setModule(module);
    box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Circle.svg")));

		addChild(createWidget<KokopelliScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<KokopelliScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<KokopelliScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<KokopelliScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addOutput(createOutput<ObservedSunPort>(mm2px(Vec(2.276, 16.435)), module, Circle::OBSERVER_OUTPUT));
		addOutput(createOutput<SunPort>(mm2px(Vec(13.65, 15.365)), module, Circle::SUN));
		addInput(createInput<BandPort>(mm2px(Vec(24.925, 16.435)), module, Circle::BAND_INPUT));

		addOutput(createOutput<PhaseOutPort>(mm2px(Vec(24.925, 29.961)), module, Circle::OBSERVER_BEAT_PHASE_OUTPUT));
		addParam(createParam<AuditionKnob>(mm2px(Vec(12.699, 28.719)), module, Circle::AUDITION_PARAM));
		addOutput(createOutput<PhaseOutPort>(mm2px(Vec(2.276, 29.961)), module, Circle::OBSERVER_PHASE_OUTPUT));

		addParam(createParam<LoveKnob>(mm2px(Vec(10.220, 76.609)), module, Circle::LOVE_PARAM));

		addParam(createParam<MediumLEDButton>(mm2px(Vec(3.127, 85.334)), module, Circle::DIVINITY_PARAM));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(4.592, 86.798)), module, Circle::DIVINITY_LIGHT));

		addParam(createParam<MediumLEDButton>(mm2px(Vec(26.850, 85.333)), module, Circle::CYCLE_PARAM));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(28.315, 86.798)), module, Circle::CYCLE_LIGHT));

		addInput(createInput<PhaseOutPort>(mm2px(Vec(13.65, 94.00)), module, Circle::PHASE_INPUT));

    auto focused_display_size = mm2px(Vec(15.736, 4.312));
		observed_sun_display = new ObservedSunDisplay(module, colors::BOX_BG_LIGHT, colors::OBSERVED_SUN, mm2px(Vec(1.236, 40.865)), focused_display_size);
		band_display = new BandDisplay(module, colors::BOX_BG_LIGHT, colors::BAND, mm2px(Vec(18.644, 40.865)), focused_display_size);

    auto beat_display_size = mm2px(Vec(7.527, 4.327));
    group_beat_display = new ObservedSunBeatDisplay(module, colors::BOX_BG_DARK, colors::OBSERVED_SUN, mm2px(Vec(1.268, 45.968)), beat_display_size);
    total_observed_sun_beats_display = new TotalObservedSunBeatDisplay(module, colors::BOX_BG_DARK, colors::OBSERVED_SUN, mm2px(Vec(9.321, 45.968)), beat_display_size);
    band_beat_display = new BandBeatDisplay(module, colors::BOX_BG_DARK, colors::BAND, mm2px(Vec(18.675, 45.968)), beat_display_size);
    total_band_beats_display = new TotalBandBeatDisplay(module, colors::BOX_BG_DARK, colors::BAND, mm2px(Vec(26.729, 45.968)), beat_display_size);

    addChild(observed_sun_display);
    addChild(band_display);
    addChild(group_beat_display);
    addChild(total_observed_sun_beats_display);
    addChild(band_beat_display);
    addChild(total_band_beats_display);
  }

	void appendContextMenu(rack::Menu* menu) override {
		auto m = dynamic_cast<Circle*>(module);
		assert(m);

    menu->addChild(new MenuLabel());

		FadeSliderItem *love_resolution_slider = new FadeSliderItem(&m->_options.love_resolution, "Love Update Frequency");
		love_resolution_slider->box.size.x = 190.f;
		menu->addChild(love_resolution_slider);

		FadeTimeSliderItem *crossfade_time_slider = new FadeTimeSliderItem(&m->_options.fade_times.crossfade, "Cycle Crossfade Time");
		crossfade_time_slider->box.size.x = 190.f;
		menu->addChild(crossfade_time_slider);

		FadeTimeSliderItem *fade_in_time_slider = new FadeTimeSliderItem(&m->_options.fade_times.fade_in, "Cycle Fade In Time ");
		fade_in_time_slider->box.size.x = 190.f;
		menu->addChild(fade_in_time_slider);

		FadeTimeSliderItem *fade_out_time_slider = new FadeTimeSliderItem(&m->_options.fade_times.fade_out, "Cycle Fade Out Time");
		fade_out_time_slider->box.size.x = 190.f;
		menu->addChild(fade_out_time_slider);

		SmoothingLambdaSliderItem *ext_phase_smoothing_lambda_slider = new SmoothingLambdaSliderItem(&m->_options.ext_phase_smoothing_lambda, "External Phase Smoothing Lambda");
		ext_phase_smoothing_lambda_slider->box.size.x = 190.f;
		menu->addChild(ext_phase_smoothing_lambda_slider);

    menu->addChild(new BoolOptionMenuItem("Cycle observed song forwards, not back", [m]() {
      return &m->_options.cycle_forward_not_back;
    }));

    menu->addChild(new BoolOptionMenuItem("Discard cycle on change return after refresh capture", [m]() {
      return &m->_options.discard_cycle_on_change_return_after_refresh;
    }));

    menu->addChild(new BoolOptionMenuItem("Attenuate captured band input at change transients", [m]() {
      return &m->_options.attenuate_captured_band_input_at_change_transients;
    }));

    menu->addChild(new BoolOptionMenuItem("Output observed song beat phase, not total song", [m]() {
      return &m->_options.output_observed_song_beat_phase_not_total_song;
    }));



    // menu->addChild(new BoolOptionMenuItem("Use anti pop filter", [m]() {
    //   return &m->_options.use_antipop_filter;
    // }));

    // menu->addChild(new BoolOptionMenuItem("Include band in total song output", [m]() {

    // menu->addChild(new BoolOptionMenuItem("Include band in total song output", [m]() {
    //   return &m->_options.include_moon_in_sun_output;
    // }));

    // menu->addChild(new BoolOptionMenuItem("Include unloved band in sun output", [m]() {
    //   return &m->_options.include_unloved_moon_in_sun_output;
    // }));

    // OptionsMenuItem::addToMenu(love_resolution, menu);

    // TODO
    // menu->addChild(new Slider());
  }
};

} // namespace kokopellivcv
