#pragma once

#include "kokopellivcv.hpp"
#include "menu.hpp"
#include "dsp/Hearth/Engine.hpp"
#include "dsp/LongPressButton.hpp"
#include "dsp/LightBlinker.hpp"
#include "widgets.hpp"
#include "util/colors.hpp"
#include "modules/HearthShared.hpp"

#include <math.h>

extern Model *modelHearth;

namespace kokopellivcv {

struct Hearth : KokopelliVcvModule {
  enum ParamIds {
		NEXT_MOVEMENT_PARAM,
		DIVINITY_PARAM,
		LOVE_PARAM,
		AUDITION_PARAM,
		NUM_PARAMS
  };
	enum InputIds {
		WOMB_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SUN,
		OBSERVER_OUTPUT,
		OBSERVER_PHASE_OUTPUT,
		OBSERVER_BEAT_PHASE_OUTPUT,
		NUM_OUTPUTS
	};

  enum LightIds {
    ENUMS(DIVINITY_LIGHT, 3),
    ENUMS(NEXT_MOVEMENT_LIGHT, 3),
    NUM_LIGHTS
  };

  float _audition_position = 1.f;

  float _sample_time = 1.0f;

  kokopellivcv::dsp::LongPressButton _next_movement_button;
  kokopellivcv::dsp::LongPressButton _voice_divinity_button;

  kokopellivcv::dsp::LightBlinker *_light_blinker;

  std::array<kokopellivcv::dsp::hearth::Engine*, maxChannels> _engines;

  rack::dsp::ClockDivider _light_divider;
  rack::dsp::ClockDivider _button_divider;

  Options _options;
  float _love_resolution = 1000.f;
  float _delay_shiftback = 0.f;

  Hearth();
  ~Hearth();

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

private:
  void processButtons(const ProcessArgs &args);
};

} // namespace kokopellivcv
