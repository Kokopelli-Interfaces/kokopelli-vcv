#pragma once

#include "Gko.hpp"

namespace myrisa {

struct GkoValueDisplay : TextBox {
	Gko *_module;
	int _previous_displayed_value = -1;

	GkoValueDisplay(Gko *m) : TextBox() {
    _module = m;
      // font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/nunito/Nunito-Bold.ttf"));
    // font = APP->window->loadFont(asset::plugin(pluginInstance, "/res/fonts/Overpass-Regular.ttf"));
    font = APP->window->loadFont(asset::plugin(pluginInstance, "/res/fonts/Nunito-Bold.ttf"));
    font_size = 12;
    letter_spacing = 0.f;
    // backgroundColor = nvgRGB(0xee, 0xe8, 0xd5); // solarized base2
    // backgroundColor = nvgRGB(0x93, 0xa1, 0xa1); // solarized base1
    // backgroundColor = nvgRGB(0x78, 0x78, 0x78); // blendish default
    // backgroundColor = nvgRGB(0xff, 0xff, 0xff); // custom
    backgroundColor = nvgRGB(0xd7, 0xda, 0xec); // blend
    textColor = defaultTextColor;
    box.size = mm2px(Vec(6.902, 3.283));
    textOffset = Vec(box.size.x * 0.5f, 0.f);
    textAlign = NVG_ALIGN_CENTER | NVG_ALIGN_TOP;
  }

	void setDisplayValue(int v) {
		std::string s;
		if(v != _previous_displayed_value) {
			_previous_displayed_value = v;
      s = string::f("%d", v);
      setText(s);
		}
	}
};

struct ActiveLayerDisplay : GkoValueDisplay {
  using GkoValueDisplay::GkoValueDisplay;
	void step() override {
		GkoValueDisplay::step();
		if(_module) {
      myrisa::dsp::gko::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }

      if (e->isRecording() || e->_new_layer_active) {
        GkoValueDisplay::setText("N");
      } else {
        int active_layer_i = e->_active_layer_i + 1;
        if (e->_timeline.layers.size() == 0) {
          active_layer_i = 0;
        }
        std::string s = string::f("%d", active_layer_i);
        GkoValueDisplay::setText(s);
      }
		}
	}
};

struct LayerBeatDisplay : GkoValueDisplay {
  using GkoValueDisplay::GkoValueDisplay;
	void step() override {
		GkoValueDisplay::step();
		if(_module) {
      myrisa::dsp::gko::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }

      int layer_beat = 0;
      if (e->isRecording()) {
        layer_beat = e->_recording_layer->getLayerBeat(e->_timeline_position.beat);
      } else if (e->_timeline.layers.size() != 0) {
        // TODO how to show start position of loops?
        layer_beat = e->_timeline.layers[e->_active_layer_i]->getLayerBeat(e->_timeline_position.beat);
      } else {
        layer_beat = -1;
      }

      GkoValueDisplay::setDisplayValue(layer_beat + 1);
		}
	}
};

struct TotalLayerBeatDisplay : GkoValueDisplay {
  using GkoValueDisplay::GkoValueDisplay;
	void step() override {
		GkoValueDisplay::step();
		if(_module) {
      myrisa::dsp::gko::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }

      int total_layer_beats = 0;
      if (e->isRecording()) {
        total_layer_beats = e->_recording_layer->_n_beats;
      } else if (e->_timeline.layers.size() != 0) {
        total_layer_beats = e->_timeline.layers[e->_active_layer_i]->_n_beats;
      }

      GkoValueDisplay::setDisplayValue(total_layer_beats);
		}
	}
};

struct CircleBeatDisplay : GkoValueDisplay {
  using GkoValueDisplay::GkoValueDisplay;
	void step() override {
		GkoValueDisplay::step();
		if(_module) {
      myrisa::dsp::gko::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }

      int circle_beat = e->_timeline_position.beat - e->_circle.first + 1;

      GkoValueDisplay::setDisplayValue(circle_beat);
		}
	}
};

struct TotalCircleBeatDisplay : GkoValueDisplay {
  using GkoValueDisplay::GkoValueDisplay;
	void step() override {
		GkoValueDisplay::step();
		if(_module) {
      myrisa::dsp::gko::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }
      unsigned int n_circle_beats = e->_circle.second - e->_circle.first;
      GkoValueDisplay::setDisplayValue(n_circle_beats);
		}
	}
};

struct TotalLayersDisplay : GkoValueDisplay {
  using GkoValueDisplay::GkoValueDisplay;
	void step() override {
		GkoValueDisplay::step();
		if(_module) {
      myrisa::dsp::gko::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }
      // TODO make me just the non attenuated layers
      int total_layers = e->_timeline.layers.size();
      GkoValueDisplay::setDisplayValue(total_layers);

		}
	}
};

struct BeatDisplay : GkoValueDisplay {
  using GkoValueDisplay::GkoValueDisplay;
	void step() override {
		GkoValueDisplay::step();
		if(_module) {
      myrisa::dsp::gko::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }
      int beat = e->_timeline_position.beat;
      GkoValueDisplay::setDisplayValue(beat+1);
		}
	}
};

struct GkoWidget : ModuleWidget {
  const int hp = 4;

  bool _use_antipop = false;

  ActiveLayerDisplay *active_layer_i;
  TotalLayersDisplay *total_layers;

  LayerBeatDisplay *layer_beat;
  TotalLayerBeatDisplay *total_layer_beats;

  CircleBeatDisplay *circle_beat;
  TotalCircleBeatDisplay *total_circle_beats;


  BeatDisplay *time;

  GkoWidget(Gko *module) {
    setModule(module);
    box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Gko.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<Rogan1HPSBrown>(mm2px(Vec(5.333, 21.157)), module, Gko::SELECT_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(17.774, 33.464)), module, Gko::SELECT_MODE_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(1.618, 33.463)), module, Gko::SELECT_FUNCTION_PARAM));

		// addParam(createParam<MediumLEDButton>(mm2px(Vec(9.64, 51.330)), module, Gko::SKIP_BACK_PARAM));
		// addParam(createParam<MediumLEDButton>(mm2px(Vec(1.447, 65.437)), module, Gko::UNFIX_BOUNDS_PARAM));
		// addParam(createParam<MediumLEDButton>(mm2px(Vec(17.849, 65.436)), module, Gko::RECORD_ON_OUTER_LOOP_PARAM));
		// addParam(createParam<Rogan3PDarkRed>(mm2px(Vec(5.334, 73.118)), module, Gko::RECORD_PARAM));

		addParam(createParam<MediumLEDButton>(mm2px(Vec(9.665, 55.94)), module, Gko::SKIP_BACK_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(1.447, 72.433)), module, Gko::UNFIX_BOUNDS_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(17.849, 72.433)), module, Gko::RECORD_ON_OUTER_LOOP_PARAM));
		addParam(createParam<Rogan3PDarkRed>(mm2px(Vec(5.334, 79.758)), module, Gko::RECORD_PARAM));

		// addInput(createInput<PJ301MPort>(mm2px(Vec(8.384, 88.869)), module, Gko::RECORD_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(8.522, 95.706)), module, Gko::RECORD_INPUT));

		addInput(createInput<PJ301MPort>(mm2px(Vec(1.798, 108.114)), module, Gko::PHASE_INPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(15.306, 108.114)), module, Gko::PHASE_OUTPUT));

		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(3.083, 34.928)), module, Gko::SELECT_FUNCTION_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(19.239, 34.928)), module, Gko::SELECT_MODE_LIGHT));
		// addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(11.155, 52.736)), module, Gko::SKIP_BACK_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(11.133, 57.346)), module, Gko::SKIP_BACK_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(11.097, 68.279)), module, Gko::RECORD_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(2.912, 73.898)), module, Gko::UNFIX_BOUNDS_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(19.313, 73.898)), module, Gko::RECORD_ON_OUTER_LOOP_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(11.181, 110.546)), module, Gko::PHASE_LIGHT));

    auto display_size = mm2px(Vec(9.096, 4.327));

		active_layer_i = new ActiveLayerDisplay(module);
    active_layer_i->box.pos = mm2px(Vec(2.921, 16.235));
    active_layer_i->box.size = display_size;
    addChild(active_layer_i);

    total_layers = new TotalLayersDisplay(module);
    total_layers->box.pos = mm2px(Vec(13.387, 16.236));
    total_layers->box.size = display_size;
    addChild(total_layers);

		// mm2px(Vec(7.391, 4.327))
    // display_size = mm2px(Vec(7.391, 4.327));
    // two displays TODO

    display_size = mm2px(Vec(23.173, 4.327));

    time = new BeatDisplay(module);
    time->box.pos = mm2px(Vec(1.088, 47.476));
    time->box.size = display_size;
    time->textOffset = Vec(time->box.size.x * 0.5f, 0.f);
    addChild(time);

    display_size = mm2px(Vec(6.837, 4.327));

    layer_beat = new LayerBeatDisplay(module);
    layer_beat->box.pos = mm2px(Vec(1.05, 52.49));
    layer_beat->box.size = display_size;
    addChild(layer_beat);

    total_layer_beats = new TotalLayerBeatDisplay(module);
    total_layer_beats->box.pos = mm2px(Vec(1.05, 57.592));
    total_layer_beats->box.size = display_size;
    addChild(total_layer_beats);

    circle_beat = new CircleBeatDisplay(module);
    circle_beat->box.pos = mm2px(Vec(17.49, 52.49));
    circle_beat->box.size = display_size;
    addChild(circle_beat);

    total_circle_beats = new TotalCircleBeatDisplay(module);
    total_circle_beats->box.pos = mm2px(Vec(17.49, 57.592));
    total_circle_beats->box.size = display_size;
    addChild(total_circle_beats);
  }

	void appendContextMenu(rack::Menu* menu) override {
		auto m = dynamic_cast<Gko*>(module);
		assert(m);

    menu->addChild(new MenuLabel());

    menu->addChild(new BoolOptionMenuItem("Use read antipop at phase discontuinity", [m]() {
      return &m->_options.use_antipop;
    }));

    menu->addChild(new BoolOptionMenuItem("Snap to divisible recording lengths", [m]() {
      return &m->_options.strict_recording_lengths;
    }));

    menu->addChild(new BoolOptionMenuItem("Create new layer on skip back", [m]() {
      return &m->_options.create_new_layer_on_skip_back;
    }));

    menu->addChild(new BoolOptionMenuItem("Bipolar Phase Input (-5V to 5V)", [m]() {
      return &m->_options.bipolar_phase_input;
    }));

    // menu->addChild(new Slider());
  }

};

} // namespace myrisa
