#pragma once

#include "Circle.hpp"

namespace kokopellivcv {

struct CircleValueDisplay : TextBox {
	Circle *_module;
	int _backward_displayed_value = -1;

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
		if(v != _backward_displayed_value) {
			_backward_displayed_value = v;
      s = string::f("%d", v);
      setText(s);
		}
	}
};


struct FocusedGroupDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
  FocusedGroupDisplay(Circle *m) : CircleValueDisplay(m) {
    textOffset = Vec(box.size.x * 0.4f, box.size.y * 0.70f);
  }

  void step() override {
		CircleValueDisplay::step();

    // FIXME
    CircleValueDisplay::setText("A");
  }
};


struct FocusedMemberDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;

	FocusedMemberDisplay(Circle *m) : CircleValueDisplay(m) {
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
      int focused_member_display = 0;
      // bool show_recording_member = true;
      bool show_recording_member = !(e->_fully_love_group && e->_record_params.tuned_to_group_frequency);
      if (show_recording_member) {
        textColor = nvgRGB(0x9b, 0x44, 0x42); // red
        // focused_member_display = e->_timeline.getNumberOfActiveMembers() + 1;
        focused_member_display = e->_timeline.members.size() + 1;
      } else {
        focused_member_display = e->_focused_member_i + 1;
        // focused_member_display = e->_timeline.getNumberOfActiveMembers();
        if (e->_timeline.members.size() == 0) {
          CircleValueDisplay::setText("--");
          CircleValueDisplay::_backward_displayed_value = -1;
          return;
        }
      }

      std::string s = string::f("%d", focused_member_display);
      CircleValueDisplay::setText(s);
		}
	}
};

struct MemberBeatDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
	void step() override {
		CircleValueDisplay::step();
		if(_module) {
      kokopellivcv::dsp::circle::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }

      int member_beat_display = 0;

      textColor = defaultTextColor;
      // bool show_recording_member = true;
      bool show_recording_member = !(e->_fully_love_group && e->_record_params.tuned_to_group_frequency);
      if (e->isRecording() && show_recording_member) {
        textColor = nvgRGB(0x9b, 0x44, 0x42); // red
        member_beat_display = e->_recording_member->getMemberBeat(e->_timeline_position.beat);

      } else if (e->_timeline.members.size() != 0) {
        // TODO how to show start position of loops?
        member_beat_display = e->_timeline.members[e->_focused_member_i]->getMemberBeat(e->_timeline_position.beat);
      } else {
        CircleValueDisplay::setText("--");
        CircleValueDisplay::_backward_displayed_value = -1;
        return;
      }

      CircleValueDisplay::setDisplayValue(member_beat_display + 1);
		}
	}
};

struct TotalMemberBeatDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
	void step() override {
		CircleValueDisplay::step();
		if(_module) {
      kokopellivcv::dsp::circle::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }

      int total_member_beats = 0;
      textColor = defaultTextColor;
      // bool show_recording_member = true;
      bool show_recording_member = !(e->_fully_love_group && e->_record_params.tuned_to_group_frequency);
      if (show_recording_member) {
        textColor = nvgRGB(0x9b, 0x44, 0x42); // red
        if (e->_record_params.tuned_to_group_frequency) {
          total_member_beats = e->_recording_member->_n_beats;
        } else if (e->_timeline.members.size() != 0) {
          total_member_beats = e->getMostRecentLoopLength();
          textColor = nvgRGB(0xe6, 0xa6, 0x0e); // yellowy
        } else {
          CircleValueDisplay::setText("--");
          CircleValueDisplay::_backward_displayed_value = -1;
          return;
        }
      } else if (e->_timeline.members.size() != 0) {
        total_member_beats = e->_timeline.members[e->_focused_member_i]->_n_beats;
      } else {
        CircleValueDisplay::setText("--");
        CircleValueDisplay::_backward_displayed_value = -1;
        return;
      }

      CircleValueDisplay::setDisplayValue(total_member_beats);
		}
	}
};

struct GroupBeatDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
	void step() override {
		CircleValueDisplay::step();
		if(_module) {
      kokopellivcv::dsp::circle::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }

      int group_beat_display = e->_timeline_position.beat - e->_circle.first + 1;

      CircleValueDisplay::setDisplayValue(group_beat_display);
		}
	}
};

struct TotalGroupBeatDisplay : CircleValueDisplay {
  using CircleValueDisplay::CircleValueDisplay;
	void step() override {
		CircleValueDisplay::step();
		if(_module) {
      kokopellivcv::dsp::circle::Engine* e = _module->_engines[0];
      if (e == NULL) {
        return;
      }
      unsigned int n_circle_beats = e->_circle.second - e->_circle.first;
      CircleValueDisplay::setDisplayValue(n_circle_beats);
		}
	}
};

struct CircleWidget : ModuleWidget {
  const int hp = 6;

  bool _use_antipop = false;

  CircleValueDisplay *prev_group_display;
  CircleValueDisplay *current_group_display;
  CircleValueDisplay *next_group_display;

  FocusedGroupDisplay *focused_group_display;
  FocusedMemberDisplay *focused_member_display;

  MemberBeatDisplay *member_beat_display;
  TotalMemberBeatDisplay *total_member_beats_display;

  GroupBeatDisplay *group_beat_display;
  TotalGroupBeatDisplay *total_group_beats_display;

  CircleWidget(Circle *module) {
    setModule(module);
    box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Circle.svg")));

		addChild(createWidget<KokopelliScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<KokopelliScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<KokopelliScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<KokopelliScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<MediumLEDButton>(mm2px(Vec(14.746, 54.933)), module, Circle::TUNE_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(3.74, 73.943)), module, Circle::BACKWARD_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(25.733, 73.943)), module, Circle::FORWARD_PARAM));
		addParam(createParam<LoveKnob>(mm2px(Vec(10.415, 78.64)), module, Circle::LOVE_PARAM));

		addInput(createInput<KokopelliPort>(mm2px(Vec(24.279, 94.783)), module, Circle::MEMBER_INPUT));
		addInput(createInput<KokopelliPort>(mm2px(Vec(13.65, 96.334)), module, Circle::LOVE_INPUT));
		addInput(createInput<KokopelliPort>(mm2px(Vec(2.986, 107.705)), module, Circle::PHASE_INPUT));
		addInput(createInput<KokopelliPort>(mm2px(Vec(13.475, 109.166)), module, Circle::FOCUS_MODULATION_INPUT));

		addOutput(createOutput<CircleOutputPort>(mm2px(Vec(13.65, 21.180)), module, Circle::CIRCLE_OUTPUT));
		addOutput(createOutput<KokopelliPort>(mm2px(Vec(2.986, 94.783)), module, Circle::GROUP_OUTPUT));
		addOutput(createOutput<KokopelliPort>(mm2px(Vec(24.216, 107.705)), module, Circle::PHASE_OUTPUT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(16.214, 56.339)), module, Circle::TUNE_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(16.178, 68.279)), module, Circle::EMERSIGN_LIGHT));

		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(5.205, 75.407)), module, Circle::BACKWARD_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(27.197, 75.407)), module, Circle::FORWARD_LIGHT));

    // NOTE do the displays too
    auto group_display_size = mm2px(Vec(8.603, 4.327));

		prev_group_display = new CircleValueDisplay(module);
    prev_group_display->box.pos = mm2px(Vec(1.326, 46.042));
    prev_group_display->box.size = group_display_size;
    addChild(prev_group_display);

		current_group_display = new CircleValueDisplay(module);
    current_group_display->box.pos = mm2px(Vec(13.47, 46.042));
    current_group_display->box.size = group_display_size;
    addChild(current_group_display);

		next_group_display = new CircleValueDisplay(module);
    next_group_display->box.pos = mm2px(Vec(25.603, 46.042));
    next_group_display->box.size = group_display_size;
    addChild(next_group_display);


    auto focused_display_size = mm2px(Vec(5.834, 9.571));

		focused_group_display = new FocusedGroupDisplay(module);
    focused_group_display->box.pos = mm2px(Vec(1.388, 52.344));
    focused_group_display->box.size = focused_display_size;
    addChild(focused_group_display);

		focused_member_display = new FocusedMemberDisplay(module);
    focused_member_display->box.pos = mm2px(Vec(28.412, 52.344));
    focused_member_display->box.size = focused_display_size;
    addChild(focused_member_display);


    auto beat_display_size = mm2px(Vec(6.385, 4.327));

    group_beat_display = new GroupBeatDisplay(module);
    group_beat_display->box.pos = mm2px(Vec(7.75, 52.344));
    group_beat_display->box.size = beat_display_size;
    addChild(group_beat_display);

    member_beat_display = new MemberBeatDisplay(module);
    member_beat_display->box.pos = mm2px(Vec(21.549, 52.344));
    member_beat_display->box.size = beat_display_size;
    addChild(member_beat_display);

    total_group_beats_display = new TotalGroupBeatDisplay(module);
    total_group_beats_display->box.pos = mm2px(Vec(7.75, 57.447));
    total_group_beats_display->box.size = beat_display_size;
    addChild(total_group_beats_display);

    total_member_beats_display = new TotalMemberBeatDisplay(module);
    total_member_beats_display->box.pos = mm2px(Vec(21.549, 57.447));
    total_member_beats_display->box.size = beat_display_size;
    addChild(total_member_beats_display);
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

		FadeSliderItem *love_resolution_slider = new FadeSliderItem(&m->_love_resolution, "Attenuation Resolution");
		love_resolution_slider->box.size.x = 190.f;
		menu->addChild(love_resolution_slider);

    // TODO
    // menu->addChild(new Slider());
  }
};

} // namespace kokopellivcv
