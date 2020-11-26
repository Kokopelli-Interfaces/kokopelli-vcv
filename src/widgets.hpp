#pragma once

#include "rack.hpp"

struct Rogan1HPSWhite : Rogan {
  Rogan1HPSWhite() {
    setSvg(APP->window->loadSvg(
        asset::system("./plugins/Myrisa/res/components/Rogan1HPSWhite.svg")));
  }
};

struct Rogan1PGray : Rogan {
  Rogan1PGray() {
    setSvg(APP->window->loadSvg(
        asset::system("./plugins/Myrisa/res/components/Rogan1PGray.svg")));
  }
};

struct Rogan4PSGray : Rogan {
  Rogan4PSGray() {
    setSvg(APP->window->loadSvg(
        asset::system("./plugins/Myrisa/res/components/Rogan4PSGray.svg")));
  }
};

struct RoganHalfPSRed : Rogan {
  RoganHalfPSRed() {
    setSvg(APP->window->loadSvg(
        asset::system("./plugins/Myrisa/res/components/RoganHalfPSRed.svg")));
  }
};

struct RoganHalfPSLightPurple : Rogan {
  RoganHalfPSLightPurple() {
    setSvg(APP->window->loadSvg(
        asset::system("./plugins/Myrisa/res/components/RoganHalfPSLightPurple.svg")));
  }
};

struct RoganHalfPGray : Rogan {
  RoganHalfPGray() {
    setSvg(APP->window->loadSvg(
        asset::system("./plugins/Myrisa/res/components/RoganHalfPGray.svg")));
  }
};

struct RoganHalfPRed : Rogan {
  RoganHalfPRed() {
    setSvg(APP->window->loadSvg(
        asset::system("./plugins/Myrisa/res/components/RoganHalfPRed.svg")));
  }
};

// same as LittleUtils ToggleLEDButton in 'Widgets.hpp'
struct ToggleLEDButton : SVGSwitch {
  BNDwidgetState state = BND_DEFAULT;
  NVGcolor defaultColor;
  NVGcolor hoverColor;

  ToggleLEDButton() {
    addFrame(APP->window->loadSvg(
        asset::system("res/ComponentLibrary/LEDButton.svg")));
  }
};

// same as LittleUtils TextBox in 'Widgets.hpp'
struct TextBox : TransparentWidget {
  // Kinda like TextField except not editable. Using Roboto Mono Bold font,
  // numbers look okay.
  // based on LedDisplayChoice
  std::string text;
  std::shared_ptr<Font> font;
  float font_size;
  float letter_spacing;
  Vec textOffset;
  NVGcolor defaultTextColor;
  NVGcolor textColor; // This can be used to temporarily override text color
  NVGcolor backgroundColor;
  int textAlign;

  TextBox() {
    // FIXME
    // font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/RobotoMono-Bold.ttf")); // TODO: fix paths...
    defaultTextColor = nvgRGB(0x23, 0x23, 0x23);
    textColor = defaultTextColor;
    backgroundColor = nvgRGB(0xc8, 0xc8, 0xc8);
    box.size = Vec(30, 18);
    // size 20 with spacing -2 will fit 3 characters on a 30px box with Roboto
    // mono
    font_size = 20;
    letter_spacing = 0.f;
    textOffset = Vec(box.size.x * 0.5f, 0.f);
    textAlign = NVG_ALIGN_CENTER | NVG_ALIGN_TOP;
  }

  virtual void setText(std::string s) { text = s; }

  virtual void draw(const DrawArgs &args) override {
    // based on LedDisplayChoice::draw() in Rack/src/app/LedDisplay.cpp
    const auto vg = args.vg;
    nvgScissor(vg, 0, 0, box.size.x, box.size.y);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0, 0, box.size.x, box.size.y, 3.0);
    nvgFillColor(vg, backgroundColor);
    nvgFill(vg);

    if (font->handle >= 0) {

      nvgFillColor(vg, textColor);
      nvgFontFaceId(vg, font->handle);

      nvgFontSize(vg, font_size);
      nvgTextLetterSpacing(vg, letter_spacing);
      nvgTextAlign(vg, textAlign);
      nvgText(vg, textOffset.x, textOffset.y, text.c_str(), NULL);
    }

    nvgResetScissor(vg);
  };
};
