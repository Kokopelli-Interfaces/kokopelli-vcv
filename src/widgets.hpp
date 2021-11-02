#pragma once

#include "rack.hpp"

#include <thread>

extern Plugin *pluginInstance;

struct MediumLEDButton : rack::app::SvgSwitch {
  MediumLEDButton() {
		momentary = true; addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/MediumLEDButton.svg")));
  }
};

struct LoveKnob : Rogan {
  LoveKnob() {
		minAngle = -0.70 * M_PI;
		maxAngle = 0.70 * M_PI;
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/LoveKnob.svg")));
  }
};

struct KokopelliPort : SvgPort {
	KokopelliPort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/KokopelliPort.svg")));
  }
};

struct CircleOutputPort : SvgPort {
	CircleOutputPort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/CircleOutputPort.svg")));
  }
};

struct KokopelliScrew : FramebufferWidget {
  SvgWidget *svg_widget;
  TransformWidget *transform_widget;

  KokopelliScrew() {
    transform_widget = new TransformWidget();
    addChild(transform_widget);

    svg_widget = new SvgWidget();
    transform_widget->addChild(svg_widget);
    svg_widget->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/KokopelliScrew.svg")));
  transform_widget->box.size = svg_widget->box.size;

    float angle = 1.71f * (rand() / (static_cast<double>(RAND_MAX) + 1.0));
    Vec transl = transform_widget->box.getCenter();
    transform_widget->translate( transl );
    transform_widget->rotate(angle);
    transform_widget->translate( transl.neg() );
  }
};

struct ToggleLEDButton : SVGSwitch {
  BNDwidgetState state = BND_DEFAULT;
  NVGcolor defaultColor;
  NVGcolor hoverColor;

  ToggleLEDButton() {
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "/LEDButton.svg")));
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

    // defaultTextColor = nvgRGB(0xae, 0x28, 0x06); // cave orange
    // defaultTextColor = nvgRGB(0xe6, 0xa6, 0x0e); // yellowy
    // defaultTextColor = nvgRGB(0x9b, 0x44, 0x42); // red
    defaultTextColor = nvgRGB(0x6e, 0xaf, 0x71); // emersign green
#
    textColor = defaultTextColor;
    backgroundColor = nvgRGB(0x2b, 0x16, 0x09); // cave #2b1609
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

// fade duration for fade slider menu item
struct FadeDuration : Quantity {
	float *srcFadeRate = NULL;
	std::string label = "";

	FadeDuration(float *_srcFadeRate, std::string fade_label) {
		srcFadeRate = _srcFadeRate;
		label = fade_label;
	}
	void setValue(float value) override {
		*srcFadeRate = math::clamp(value, getMinValue(), getMaxValue());
	}
	float getValue() override {
		return *srcFadeRate;
	}
	float getMinValue() override {return 26.0f;}
	float getMaxValue() override {return 44100.0f * 2;}
	float getDefaultValue() override {return 20000.0f;}
	float getDisplayValue() override {return getValue() / 44100;}
	std::string getDisplayValueString() override {
		float value = getDisplayValue();
		return string::f("%.2f", value);
	}
	void setDisplayValue(float displayValue) override {setValue(displayValue);}
	std::string getLabel() override {return label;}
	std::string getUnit() override {return " sec";}
};


// fade automation sliders
struct FadeSliderItem : ui::Slider {
	FadeSliderItem(float *fade_rate, std::string fade_label) {
		quantity = new FadeDuration(fade_rate, fade_label);
	}

	~FadeSliderItem() {
		delete quantity;
	}
};

struct BlinkableRedGreenBlueLight : RedGreenBlueLight {
  std::chrono::time_point<std::chrono::system_clock> blink;
  bool op = true;

  BlinkableRedGreenBlueLight() {
    blink = std::chrono::system_clock::now();
  }

  void blinkLight() {
    op = true;
  }

  void step() override {
    if (module) {
      auto now = std::chrono::system_clock::now();
      if (now - blink > std::chrono::milliseconds{300}) {
        op = false;
        blink = now;
      }

      std::vector<float> brightnesses(baseColors.size());
      for (size_t i = 0; i < baseColors.size(); i++) {
        float b = module->lights[firstLightId + i].getBrightness();
        if (b > 0.f) {
          b = op ? 1.f : 0.6f;
        }
        brightnesses[i] = b;
      }
      setBrightnesses(brightnesses);
    }
  }
};
