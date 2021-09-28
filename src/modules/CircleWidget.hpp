#pragma once

#include "Circle.hpp"

namespace kokopelli {

struct CircleValueDisplay : TextBox {
	Circle *_module;
	int _previous_member_displayed_value = -1;

	CircleValueDisplay(Circle *m) : TextBox() {
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
    // backgroundColor = nvgRGB(0xd7, 0xda, 0xec); // blend
    backgroundColor = nvgRGB(0x2b, 0x16, 0x09); // cave #2b1609
    textColor = defaultTextColor;
    box.size = mm2px(Vec(6.902, 3.283));
    textOffset = Vec(box.size.x * 0.5f, 0.f);
    textAlign = NVG_ALIGN_CENTER | NVG_ALIGN_TOP;
  }

	void setDisplayValue(int v) {
		std::string s;
		if(v != _previous_member_displayed_value) {
			_previous_member_displayed_value = v;
      s = string::f("%d", v);
      setText(s);
		}
	}
};

struct ActiveMemberDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
	void step() override {
		CircleValueDisplay::step();
		if(_module) {
      kokopelli::dsp::circle::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }

      if (e->isLoving() || e->_new_member_active) {
        CircleValueDisplay::setText("N");
      } else {
        int active_member_i = e->_active_member_i + 1;
        if (e->_circle.members.size() == 0) {
          active_member_i = 0;
        }
        std::string s = string::f("%d", active_member_i);
        CircleValueDisplay::setText(s);
      }
		}
	}
};

struct MemberBeatDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
	void step() override {
		CircleValueDisplay::step();
		if(_module) {
      kokopelli::dsp::circle::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }

      int member_beat = 0;
      if (e->_recording_member != NULL && e->isLoving()) {
        member_beat = e->_recording_member->getMemberBeat(e->_circle_position.beat);
      } else if (e->_circle.members.size() != 0) {
        // TODO how to show start position of loops?
        member_beat = e->_circle.members[e->_active_member_i]->getMemberBeat(e->_circle_position.beat);
      } else {
        member_beat = -1;
      }

      CircleValueDisplay::setDisplayValue(member_beat + 1);
		}
	}
};

struct TotalMemberBeatDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
	void step() override {
		CircleValueDisplay::step();
		if(_module) {
      kokopelli::dsp::circle::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }

      int total_member_beats = 0;
      if (e->isLoving()) {
        total_member_beats = e->_recording_member->_n_beats;
      } else if (e->_circle.members.size() != 0) {
        total_member_beats = e->_circle.members[e->_active_member_i]->_n_beats;
      }

      CircleValueDisplay::setDisplayValue(total_member_beats);
		}
	}
};

struct CircleBeatDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
	void step() override {
		CircleValueDisplay::step();
		if(_module) {
      kokopelli::dsp::circle::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }

      int circle_beat = e->_circle_position.beat - e->_loop.first + 1;

      CircleValueDisplay::setDisplayValue(circle_beat);
		}
	}
};

struct TotalCircleBeatDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
	void step() override {
		CircleValueDisplay::step();
		if(_module) {
      kokopelli::dsp::circle::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }
      unsigned int n_circle_beats = e->_loop.second - e->_loop.first;
      CircleValueDisplay::setDisplayValue(n_circle_beats);
		}
	}
};

struct TotalMembersDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
	void step() override {
		CircleValueDisplay::step();
		if(_module) {
      kokopelli::dsp::circle::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }
      // TODO make me just the non attenuated members
      int total_members = e->_circle.members.size();
      CircleValueDisplay::setDisplayValue(total_members);

		}
	}
};

struct BeatDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
	void step() override {
		CircleValueDisplay::step();
		if(_module) {
      kokopelli::dsp::circle::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }
      int beat = e->_circle_position.beat;
      CircleValueDisplay::setDisplayValue(beat+1);
		}
	}
};

struct CircleWidget : ModuleWidget {
  const int hp = 4;

  bool _use_antipop = false;

  ActiveMemberDisplay *active_member_i;
  TotalMembersDisplay *total_members;

  MemberBeatDisplay *member_beat;
  TotalMemberBeatDisplay *total_member_beats;

  CircleBeatDisplay *circle_beat;
  TotalCircleBeatDisplay *total_circle_beats;


  BeatDisplay *time;

  CircleWidget(Circle *module) {
    setModule(module);
    box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Circle.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<CircleFocusKnob>(mm2px(Vec(5.333, 21.157)), module, Circle::SELECT_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(17.774, 33.464)), module, Circle::SELECT_MODE_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(1.618, 33.463)), module, Circle::SELECT_FUNCTION_PARAM));

		// addParam(createParam<MediumLEDButton>(mm2px(Vec(9.64, 51.330)), module, Circle::REFLECT_PARAM));
		// addParam(createParam<MediumLEDButton>(mm2px(Vec(1.447, 65.437)), module, Circle::PREV_MEMBER_PARAM));
		// addParam(createParam<MediumLEDButton>(mm2px(Vec(17.849, 65.436)), module, Circle::NEXT_MEMBER_PARAM));
		// addParam(createParam<Rogan3PDarkRed>(mm2px(Vec(5.334, 73.118)), module, Circle::LOVE_PARAM));

		addParam(createParam<MediumLEDButton>(mm2px(Vec(9.665, 55.94)), module, Circle::REFLECT_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(1.447, 72.433)), module, Circle::PREV_MEMBER_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(17.849, 72.433)), module, Circle::NEXT_MEMBER_PARAM));
		addParam(createParam<Rogan3PDarkRed>(mm2px(Vec(5.334, 79.758)), module, Circle::LOVE_PARAM));

		// addInput(createInput<PJ301MPort>(mm2px(Vec(8.384, 88.869)), module, Circle::RECORD_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(8.522, 95.706)), module, Circle::RECORD_INPUT));

		addInput(createInput<PJ301MPort>(mm2px(Vec(1.798, 108.114)), module, Circle::PHASE_INPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(15.306, 108.114)), module, Circle::PHASE_OUTPUT));

		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(3.083, 34.928)), module, Circle::SELECT_FUNCTION_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(19.239, 34.928)), module, Circle::SELECT_MODE_LIGHT));
		// addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(11.155, 52.736)), module, Circle::REFLECT_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(11.133, 57.346)), module, Circle::REFLECT_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(11.097, 68.279)), module, Circle::RECORD_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(2.912, 73.898)), module, Circle::PREVIOUS_MEMBER_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(19.313, 73.898)), module, Circle::NEXT_MEMBER_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(11.181, 110.546)), module, Circle::PHASE_LIGHT));

    auto display_size = mm2px(Vec(9.096, 4.327));

		active_member_i = new ActiveMemberDisplay(module);
    active_member_i->box.pos = mm2px(Vec(2.921, 16.235));
    active_member_i->box.size = display_size;
    addChild(active_member_i);

    total_members = new TotalMembersDisplay(module);
    total_members->box.pos = mm2px(Vec(13.387, 16.236));
    total_members->box.size = display_size;
    addChild(total_members);

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

    member_beat = new MemberBeatDisplay(module);
    member_beat->box.pos = mm2px(Vec(1.05, 52.49));
    member_beat->box.size = display_size;
    addChild(member_beat);

    total_member_beats = new TotalMemberBeatDisplay(module);
    total_member_beats->box.pos = mm2px(Vec(1.05, 57.592));
    total_member_beats->box.size = display_size;
    addChild(total_member_beats);

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
		auto m = dynamic_cast<Circle*>(module);
		assert(m);

    menu->addChild(new MenuLabel());

    menu->addChild(new BoolOptionMenuItem("Use read antipop at phase discontuinity", [m]() {
      return &m->_options.use_antipop;
    }));

    menu->addChild(new BoolOptionMenuItem("Snap to divisible recording lengths", [m]() {
      return &m->_options.strict_recording_lengths;
    }));

    menu->addChild(new BoolOptionMenuItem("Create new member on skip back", [m]() {
      return &m->_options.create_new_member_on_reflect;
    }));

    menu->addChild(new BoolOptionMenuItem("Bipolar Phase Input (-5V to 5V)", [m]() {
      return &m->_options.bipolar_phase_input;
    }));

    // menu->addChild(new Slider());
  }

};

} // namespace kokopelli
