#pragma once

#include "kokopellivcv.hpp"
#include "Circle.hpp"
#include "menu.hpp"
#include "dsp/Circle/Engine.hpp"
#include "dsp/LongPressButton.hpp"
#include "dsp/LightBlinker.hpp"
#include "widgets.hpp"

#include <math.h>

extern Model *modelCircle;

namespace kokopellivcv {

struct Circle : KokopelliVcvModule {
  enum ParamIds {
		MODE_PARAM,
		NEXT_PARAM,
		PREVIOUS_PARAM,
		LOVE_PARAM,
		NUM_PARAMS
  };
	enum InputIds {
		MEMBER_INPUT,
		LOVE_INPUT,
		PHASE_INPUT,
		FOCUS_MODULATION_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CIRCLE_OUTPUT,
		GROUP_OUTPUT,
		PHASE_OUTPUT,
		NUM_OUTPUTS
	};

  enum LightIds {
    ENUMS(MODE_LIGHT, 3),
    ENUMS(EMERSIGN_LIGHT, 3),
    ENUMS(PREVIOUS_LIGHT, 3),
    ENUMS(NEXT_LIGHT, 3),
    NUM_LIGHTS
  };

  float _sampleTime = 1.0f;

  kokopellivcv::dsp::LongPressButton _mode_button;
  kokopellivcv::dsp::LongPressButton _next_button;
  kokopellivcv::dsp::LongPressButton _previous_button;

  kokopellivcv::dsp::LightBlinker *_light_blinker;

  std::array<kokopellivcv::dsp::circle::Engine*, maxChannels> _engines;

  rack::dsp::ClockDivider _light_divider;
  rack::dsp::ClockDivider _button_divider;

  float _attenuation_resolution = 10000.f;
  Options _options;

  Circle();
  ~Circle();

  void sampleRateChange() override;
  int channels() override;
  void modulate() override;
  void modulateChannel(int c) override;
  void addChannel(int c) override;
  void removeChannel(int c) override;
  void processAlways(const ProcessArgs &args) override;
  void processChannel(const ProcessArgs &args, int channel) override;
  void postProcessAlways(const ProcessArgs &args) override;
  void updateLights(const ProcessArgs &args);
  void updateLight(int light, float r, float g, float b);

private:
  void processButtons(const ProcessArgs &args);
};

} // namespace kokopellivcv

