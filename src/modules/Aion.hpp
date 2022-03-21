#pragma once

#include "kokopellivcv.hpp"
#include "dsp/LongPressButton.hpp"
#include "dsp/LightBlinker.hpp"
#include "modules/Hearth.hpp"
#include "widgets.hpp"
#include "util/colors.hpp"
#include "modules/HearthShared.hpp"

extern Model *modelAion;

namespace kokopellivcv {

struct Aion : KokopelliVcvModule {
	enum ParamIds {
		PREV_MOVEMENT_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		INPUTS_LEN,
		NUM_INPUTS
	};
	enum OutputId {
		NUM_OUTPUTS
	};
  enum LightIds {
    ENUMS(PREV_MOVEMENT_LIGHT, 3),
    NUM_LIGHTS
	};

  Hearth* _connected_hearth = nullptr;
  float _sample_time = 1.f;

  kokopellivcv::dsp::LongPressButton _prev_movement_button;
  kokopellivcv::dsp::LightBlinker *_light_blinker;

  rack::dsp::ClockDivider _light_divider;
  rack::dsp::ClockDivider _button_divider;

  Aion();

  void sampleRateChange() override;
  // int channels() override;
  // void modulate() override;
  // void modulateChannel(int c) override;
  // void addChannel(int c) override;
  // void removeChannel(int c) override;
  void processAlways(const ProcessArgs &args) override;
  // void processChannel(const ProcessArgs &args, int channel) override;
  void postProcessAlways(const ProcessArgs &args) override;
  void updateLights(const ProcessArgs &args);
  void updateLight(int light, NVGcolor color, float strength);

private:
  void connect();
  bool isNextToHearth();
  void processButtons(const ProcessArgs &args);
};

} // namespace kokopellivcv

