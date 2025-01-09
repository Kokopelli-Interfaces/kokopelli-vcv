#pragma once

#include "rack.hpp"

#include <thread>

extern Plugin *pluginInstance;

struct MediumLEDButton : rack::app::SvgSwitch {
  MediumLEDButton() {
		momentary = true;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/MediumLEDButton.svg")));
  }
};

struct AuditionKnob : Rogan {
  AuditionKnob() {
    speed = 0.75f;
		minAngle = -0.25 * M_PI;
		maxAngle = 0.25 * M_PI;
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/AuditionKnob.svg")));
  }
};

struct LoveKnob : Rogan {
  LoveKnob() {
    speed = 0.5f;
		minAngle = -0.25 * M_PI;
		maxAngle = 0.25 * M_PI;
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/LoveKnob.svg")));
  }
};

struct KokopelliPort : SvgPort {
	KokopelliPort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/KokopelliPort.svg")));
  }
};

struct ObservedSunPort : SvgPort {
	ObservedSunPort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/ObservedSunPort.svg")));
  }
};


struct SunPort : SvgPort {
	SunPort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/SunPort.svg")));
  }
};

struct PhasePort : SvgPort {
	PhasePort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/PhasePort.svg")));
  }
};

struct PhaseOutPort : SvgPort {
	PhaseOutPort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/PhaseOutPort.svg")));
  }
};

struct PhaseInPort : SvgPort {
	PhaseInPort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/PhaseInPort.svg")));
  }
};

struct BandPort : SvgPort {
	BandPort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BandPort.svg")));
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

// same as LittleUtils TextBox in 'Widgets.hpp'
struct TextBox : TransparentWidget {
  // Kinda like TextField except not editable. Using Roboto Mono Bold font,
  // numbers look okay.
  // based on LedDisplayChoice
  std::string text;
  std::string fontPath;
  std::shared_ptr<Font> font;
  float font_size;
  float letter_spacing;
  Vec textOffset;
  NVGcolor defaultTextColor;
  NVGcolor textColor; // This can be used to temporarily override text color
  NVGcolor backgroundColor;
  // int textAlign;

  TextBox() {
    defaultTextColor = nvgRGB(0x19, 0x0d, 0x05); // observed_sun
    textColor = defaultTextColor;
    backgroundColor = nvgRGB(0x19, 0x0d, 0x05);
    font_size = 20;
    letter_spacing = 0.f;
    textOffset = Vec(box.size.x * 0.5f, 0.f);
  }

  virtual void setText(std::string s) { text = s; }

  virtual void draw(const DrawArgs &args) override {
    // based on LedDisplayChoice::draw() in Rack/src/app/LedDisplay.cpp
    auto vg = args.vg;
    nvgScissor(vg, 0, 0, box.size.x, box.size.y);

    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0, 0, box.size.x, box.size.y, 3.0);
    nvgFillColor(vg, backgroundColor);
    nvgFill(vg);

    font = APP->window->loadFont(asset::plugin(pluginInstance, fontPath));

    if (font->handle >= 0) {

      nvgFillColor(vg, textColor);
      nvgFontFaceId(vg, font->handle);

      nvgFontSize(vg, font_size);
      nvgTextLetterSpacing(vg, letter_spacing);
      nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
      nvgText(vg, textOffset.x, textOffset.y, text.c_str(), NULL);
    }

    nvgResetScissor(vg);
  };
};

// fade duration for fade slider menu item
struct FadeDuration : Quantity {
	float *src = NULL;
	std::string label = "";

	FadeDuration(float *_src, std::string fade_label) {
		src = _src;
		label = fade_label;
	}
	void setValue(float value) override {
		*src = math::clamp(value, getMinValue(), getMaxValue());
	}
	float getValue() override {
		return *src;
	}
	float getMinValue() override {return 26.0f;}
	float getMaxValue() override {return 44100.0f * 2;}
	float getDefaultValue() override {return 10000.0f;}
	float getDisplayValue() override {return getValue() / (44100 / 100);}
	std::string getDisplayValueString() override {
		float value = getDisplayValue();
		return string::f("%.0f", value);
	}
	void setDisplayValue(float displayValue) override {setValue(displayValue);}
	std::string getLabel() override {return label;}
	std::string getUnit() override {return " ms";}
};

struct FadeSliderItem : ui::Slider {
	FadeSliderItem(float *fade_rate, std::string fade_label) {
		quantity = new FadeDuration(fade_rate, fade_label);
	}

	~FadeSliderItem() {
		delete quantity;
	}
};

struct FadeTimeMult : Quantity {
	float *src = NULL;
	std::string label = "";

	FadeTimeMult(float *_src, std::string fade_label) {
		src = _src;
		label = fade_label;
	}
	void setValue(float value) override {
		*src = math::clamp(value, getMinValue(), getMaxValue());
	}
	float getValue() override {
		return *src;
	}
	float getMinValue() override {return 0.0f;}
	float getMaxValue() override {return 100.f;}
	float getDefaultValue() override {return 1.0f;}
	std::string getLabel() override {return label;}
};

struct FadeTimeMultSliderItem : ui::Slider {
	FadeTimeMultSliderItem(float *fade_mult, std::string fade_label) {
		quantity = new FadeTimeMult(fade_mult, fade_label);
	}

	~FadeTimeMultSliderItem() {
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
