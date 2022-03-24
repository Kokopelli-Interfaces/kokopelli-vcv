#pragma once

#include "Aion.hpp"

#include "util/colors.hpp"
#include "util/roman.hpp"

#include <iostream>

namespace kokopellivcv {

static std::string ROMAN_NUMERALS[] = {"0", "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X", "XI", "XII", "XIII", "XIV", "XV", "XVI", "XVII", "XVIII", "XIX", "XX", "XXI", "XXII", "XXIII", "XXIV", "XXV", "XXVI", "XXVII", "XXVIII", "XXIX", "XXX", "XXXI", "XXXII", "XXXIII", "XXXIV", "XXXV", "XXXVI", "XXXVII", "XXXVIII", "XXXIX", "XL", "XLI", "XLII", "XLIII", "XLIV", "XLV", "XLVI", "XLVII", "XLVIII", "XLIX", "L", "LI", "LII", "LIII", "LIV", "LV", "LVI", "LVII", "LVIII", "LIX", "LX", "LXI", "LXII", "LXIII", "LXIV", "LXV", "LXVI", "LXVII", "LXVIII", "LXIX", "LXX", "LXXI", "LXXII", "LXXIII", "LXXIV", "LXXV", "LXXVI", "LXXVII", "LXXVIII", "LXXIX", "LXXX", "LXXXI", "LXXXII", "LXXXIII", "LXXXIV", "LXXXV", "LXXXVI", "LXXXVII", "LXXXVIII", "LXXXIX", "XC", "XCI", "XCII", "XCIII", "XCIV", "XCV", "XCVI", "XCVII", "XCVIII", "XCIX", "C"};

struct AionTextBox : TextBox {
	Aion *_module = nullptr;
	int _previous_displayed_value = -1;

	AionTextBox(Aion *m, NVGcolor background_color, NVGcolor text_color, rack::math::Vec pos, rack::math::Vec size) : TextBox() {
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
    if (!_module || !_module->_connected_hearth || _module->_connected_hearth->channels() == 0) {
      return;
    }

    kokopellivcv::dsp::hearth::Engine* e = _module->_connected_hearth->_engines[0];
    update(e);
  }

  virtual void update(kokopellivcv::dsp::hearth::Engine* engine) {
    return;
  };

  // TODO optional roman numerals
	void setMovementNumber(unsigned int movement_n) {
		std::string s;
		if(movement_n != (unsigned int)_previous_displayed_value) {
			_previous_displayed_value = movement_n;

      std::string roman_numeral;
      if (100 < movement_n) {
        roman_numeral = kokopellivcv::util::intToRomanNumeral(movement_n);
      } else {
        roman_numeral = ROMAN_NUMERALS[movement_n];
      }

      setText(roman_numeral);
		}
	}
};

struct ParentGroupDisplay : AionTextBox {
  using AionTextBox::AionTextBox;

  void update(kokopellivcv::dsp::hearth::Engine* e) override {
    if (e->_village.conductor.focus_group->parent_group) {
      std::string s = e->_village.conductor.focus_group->parent_group->name;
      AionTextBox::setText(s);
    } else {
      AionTextBox::setText("--");
    }
  }
};

struct GroupDisplay : AionTextBox {
  using AionTextBox::AionTextBox;

  void update(kokopellivcv::dsp::hearth::Engine* e) override {
    std::string s = e->_village.conductor.focus_group->name;
    AionTextBox::setText(s);
  }
};

struct PrevPrevMovementDisplay : AionTextBox {
  using AionTextBox::AionTextBox;

  void update(kokopellivcv::dsp::hearth::Engine* e) override {
    unsigned int group_movement_n = e->_village.conductor.focus_group->getMostRecentMovement(-2);
    if (group_movement_n == 0) {
      textColor = colors::DIM_SUN;
      AionTextBox::setText("--");
    } else {
      textColor = colors::FOCUS_GROUP;
      AionTextBox::setMovementNumber(group_movement_n);
    }
  }
};


struct PrevMovementDisplay : AionTextBox {
  using AionTextBox::AionTextBox;

  void update(kokopellivcv::dsp::hearth::Engine* e) override {
    unsigned int group_movement_n = e->_village.conductor.focus_group->getMostRecentMovement(-1);
    if (group_movement_n == 0) {
      textColor = colors::DIM_SUN;
      AionTextBox::setText("--");
    } else {
      textColor = colors::FOCUS_GROUP;
      AionTextBox::setMovementNumber(group_movement_n);
    }
  }
};

struct CurrentMovementDisplay : AionTextBox {
  using AionTextBox::AionTextBox;

  void update(kokopellivcv::dsp::hearth::Engine* e) override {
    float movement_phase = e->_village.conductor.focus_group->getMostRecentMovementPhase();
    backgroundColor.a = movement_phase;

    unsigned int group_movement_n = e->_village.conductor.focus_group->getMostRecentMovement(0);
    if (group_movement_n == 0) {
      textColor = colors::DIM_SUN;
      AionTextBox::setText("--");
    } else {
      textColor = colors::FOCUS_GROUP;
      AionTextBox::setMovementNumber(group_movement_n);
    }

  }
};

struct NextMovementDisplay : AionTextBox {
  using AionTextBox::AionTextBox;

  void update(kokopellivcv::dsp::hearth::Engine* e) override {
    unsigned int group_movement_n = e->_village.conductor.focus_group->getMostRecentMovement(1);

    if (group_movement_n == 1) {
      textColor = colors::DIM_SUN;
      AionTextBox::setMovementNumber(1);
    } else if (group_movement_n == 0) {
      textColor = colors::DIM_SUN;
      AionTextBox::setText("--");
    } else {
      textColor = colors::FOCUS_GROUP;
      AionTextBox::setMovementNumber(group_movement_n);
    }
  }
};

struct NextNextMovementDisplay : AionTextBox {
  using AionTextBox::AionTextBox;

  void update(kokopellivcv::dsp::hearth::Engine* e) override {
    unsigned int group_movement_n = e->_village.conductor.focus_group->getMostRecentMovement(2);

    if (group_movement_n == 2) {
      textColor = colors::DIM_SUN;
      AionTextBox::setMovementNumber(2);
    } else if (group_movement_n == 0) {
      textColor = colors::DIM_SUN;
      AionTextBox::setText("--");
    } else {
      textColor = colors::FOCUS_GROUP;
      AionTextBox::setMovementNumber(group_movement_n);
    }
  }
};



struct AionWidget : ModuleWidget {
  const int hp = 4;

  ParentGroupDisplay *_parent_group_display;
  GroupDisplay *_group_display;

  NextNextMovementDisplay *_next_next_movement_display;
  NextMovementDisplay *_next_movement_display;
  CurrentMovementDisplay *_current_movement_display;
  PrevMovementDisplay *_prev_movement_display;
  PrevPrevMovementDisplay *_prev_prev_movement_display;

	AionWidget(Aion* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Aion.svg")));

		addChild(createWidget<KokopelliScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<KokopelliScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<KokopelliScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<KokopelliScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<MediumLEDButton>(mm2px(Vec(8.183, 91.694)), module, Aion::PREV_MOVEMENT_PARAM));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(9.648, 93.157)), module, Aion::PREV_MOVEMENT_LIGHT));

    auto top_display_size = mm2px(Vec(19.472, 4.312));
    _parent_group_display = new ParentGroupDisplay(module, colors::BOX_BG_DARK, colors::FOCUS_GROUP, mm2px(Vec(2.531, 14.862)), top_display_size);
    _group_display = new GroupDisplay(module, colors::BOX_BG_DARK, colors::FOCUS_GROUP, mm2px(Vec(2.531, 20.550)), top_display_size);

    auto movement_display_size = mm2px(Vec(11.735, 4.327));

		_prev_prev_movement_display = new PrevPrevMovementDisplay(module, colors::BOX_BG_DARK, colors::FOCUS_GROUP, mm2px(Vec(9.403, 51.468)), movement_display_size);

    _prev_movement_display = new PrevMovementDisplay(module, colors::BOX_BG_DARK, colors::FOCUS_GROUP, mm2px(Vec(6.020, 59.131)), movement_display_size);

    auto current_movement_display_size = mm2px(Vec(15.736, 4.312));
    _current_movement_display = new CurrentMovementDisplay(module, colors::BOX_BG_DARK, colors::FOCUS_GROUP, mm2px(Vec(3.604, 67.484)), current_movement_display_size);

    _next_movement_display = new NextMovementDisplay(module, colors::BOX_BG_DARK, colors::FOCUS_GROUP, mm2px(Vec(6.020, 75.551)), movement_display_size);

    _next_next_movement_display = new NextNextMovementDisplay(module, colors::BOX_BG_DARK, colors::FOCUS_GROUP, mm2px(Vec(9.403, 83.562)), movement_display_size);

    addChild(_parent_group_display);
    addChild(_group_display);
    addChild(_next_next_movement_display);
    addChild(_next_movement_display);
    addChild(_current_movement_display);
    addChild(_prev_movement_display);
    addChild(_prev_prev_movement_display);
	}
};

} // namespace kokopellivcv
