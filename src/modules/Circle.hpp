#pragma once

#include "kokopellivcv.hpp"
#include "Circle.hpp"
#include "menu.hpp"
#include "dsp/Circle/Engine.hpp"
#include "dsp/LongPressButton.hpp"
#include "dsp/LightBlinker.hpp"
#include "widgets.hpp"
#include "util/colors.hpp"

#include <math.h>

extern Model *modelCircle;

namespace kokopellivcv {

struct Circle : KokopelliVcvModule {
  enum ParamIds {
		CYCLE_PARAM,
		DIVINITY_PARAM,
		LOVE_PARAM,
		AUDITION_PARAM,
		NUM_PARAMS
  };
	enum InputIds {
		WOMB_INPUT,
		PHASE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SUN,
		ESTABLISHED_OUTPUT,
		PHASE_OUTPUT,
		NUM_OUTPUTS
	};

  enum LightIds {
    ENUMS(DIVINITY_LIGHT, 3),
    ENUMS(CYCLE_LIGHT, 3),
    NUM_LIGHTS
  };

  float _audition_position = 1.f;

  float _sample_time = 1.0f;

  kokopellivcv::dsp::LongPressButton _cycle_forward_button;
  kokopellivcv::dsp::LongPressButton _cycle_divinity_button;

  kokopellivcv::dsp::LightBlinker *_light_blinker;

  std::array<kokopellivcv::dsp::circle::Engine*, maxChannels> _engines;

  rack::dsp::ClockDivider _light_divider;
  rack::dsp::ClockDivider _button_divider;

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
  void updateLight(int light, NVGcolor color, float strength);

  void updateLoveResolution();

private:
  void processButtons(const ProcessArgs &args);
};

} // namespace kokopellivcv

