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
    if (!_module || !_module->_connected_circle || _module->_connected_circle->channels() == 0) {
      return;
    }

    kokopellivcv::dsp::circle::Engine* e = _module->_connected_circle->_engines[0];
    update(e);
  }

  virtual void update(kokopellivcv::dsp::circle::Engine* engine) {
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

struct SongDisplay : AionTextBox {
  using AionTextBox::AionTextBox;

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    std::string s = e->_song.name;
    AionTextBox::setText(s);
  }
};


struct PrevMovementDisplay : AionTextBox {
  using AionTextBox::AionTextBox;

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    unsigned int group_movement_n = e->_song.current_movement->group_movement_n;
    if (e->_song.current_movement->prev) {
      textColor = colors::OBSERVED_SUN;
      AionTextBox::setMovementNumber(group_movement_n-1);
    } else {
      textColor = colors::DIM_SUN;
      AionTextBox::setText("--");
    }
  }
};

struct CurrentMovementDisplay : AionTextBox {
  using AionTextBox::AionTextBox;

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    unsigned int group_movement_n = e->_song.current_movement->group_movement_n;
    AionTextBox::setMovementNumber(group_movement_n);
  }
};

struct NextMovementDisplay : AionTextBox {
  using AionTextBox::AionTextBox;

  void update(kokopellivcv::dsp::circle::Engine* e) override {
    unsigned int group_movement_n = e->_song.current_movement->group_movement_n;
    if (e->_song.current_movement->next) {
      textColor = colors::OBSERVED_SUN;
      AionTextBox::setMovementNumber(group_movement_n+1);
    } else if (group_movement_n != 1) {
      textColor = colors::DIM_SUN;
      AionTextBox::setMovementNumber(1);
    } else {
      textColor = colors::DIM_SUN;
      AionTextBox::setText("--");
    }
  }
};


struct AionWidget : ModuleWidget {
  const int hp = 4;

  AionTextBox *_aion_display;
  SongDisplay *_song_display;

  AionTextBox *_next_next_movement_display;
  NextMovementDisplay *_next_movement_display;
  CurrentMovementDisplay *_current_movement_display;
  PrevMovementDisplay *_prev_movement_display;
  AionTextBox *_prev_prev_movement_display;

  // SongDisplay *song_display;
  // PrevMovementDisplay *prev_movement_display;
  // CurrentMovementDisplay *current_movement_display;
  // NextMovementDisplay *next_movement_display;

	AionWidget(Aion* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Aion.svg")));

		addChild(createWidget<KokopelliScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<KokopelliScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<KokopelliScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<KokopelliScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<MediumLEDButton>(mm2px(Vec(8.183, 41.077)), module, Aion::CYCLE_BACKWARD_PARAM));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(9.648, 42.541)), module, Aion::CYCLE_BACKWARD_LIGHT));

    auto top_display_size = mm2px(Vec(19.472, 4.312));
    _aion_display = new AionTextBox(module, colors::BOX_BG_DARK, colors::OBSERVED_SUN, mm2px(Vec(2.531, 14.862)), top_display_size);
    _song_display = new SongDisplay(module, colors::BOX_BG_DARK, colors::OBSERVED_SUN, mm2px(Vec(2.531, 20.550)), top_display_size);

    auto movement_display_size = mm2px(Vec(11.735, 4.327));

		_next_next_movement_display = new AionTextBox(module, colors::BOX_BG_DARK, colors::OBSERVED_SUN, mm2px(Vec(9.403, 51.468)), movement_display_size);

    _next_movement_display = new NextMovementDisplay(module, colors::BOX_BG_DARK, colors::OBSERVED_SUN, mm2px(Vec(6.020, 59.131)), movement_display_size);

    auto current_movement_display_size = mm2px(Vec(15.736, 4.312));
    _current_movement_display = new CurrentMovementDisplay(module, colors::BOX_BG_DARK, colors::OBSERVED_SUN, mm2px(Vec(3.604, 67.484)), current_movement_display_size);

    _prev_movement_display = new PrevMovementDisplay(module, colors::BOX_BG_DARK, colors::OBSERVED_SUN, mm2px(Vec(6.020, 75.551)), movement_display_size);

    _prev_prev_movement_display = new AionTextBox(module, colors::BOX_BG_DARK, colors::OBSERVED_SUN, mm2px(Vec(9.403, 83.562)), movement_display_size);

    addChild(_aion_display);
    addChild(_song_display);
    addChild(_next_next_movement_display);
    addChild(_next_movement_display);
    addChild(_current_movement_display);
    addChild(_prev_movement_display);
    addChild(_prev_prev_movement_display);
	}
};

} // namespace kokopellivcv
