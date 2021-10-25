#pragma once

#include "Circle_shared.hpp"
#include "menu.hpp"
#include "dsp/Circle/Engine.hpp"
#include "dsp/LongPressButton.hpp"
#include "widgets.hpp"
#include <math.h>

namespace kokopellivcv {

struct Circle : ExpanderModule<SignalExpanderMessage, KokopelliVcvModule> {
  enum ParamIds {
    SELECT_PARAM,
    SELECT_MODE_PARAM,
    SELECT_FUNCTION_PARAM,
    LOOP_PARAM,
    FORGET_PARAM,
    FIX_BOUNDS_PARAM,
    RECORD_PARAM,
    NUM_PARAMS
  };
  enum InputIds { SCENE_INPUT, RECORD_INPUT, PHASE_INPUT, NUM_INPUTS };
  enum OutputIds { PHASE_OUTPUT, NUM_OUTPUTS };
  enum LightIds {
    ENUMS(SELECT_FUNCTION_LIGHT, 3),
    ENUMS(SELECT_MODE_LIGHT, 3),
    ENUMS(SKIP_BACK_LIGHT, 3),
    ENUMS(RECORD_LIGHT, 3),
    ENUMS(FIX_BOUNDS_LIGHT, 3),
    ENUMS(FORGET_LIGHT, 3),
    ENUMS(PHASE_LIGHT, 3),
    NUM_LIGHTS
  };

  SignalExpanderMessage *_to_signal = nullptr;
  SignalExpanderMessage *_from_signal = nullptr;

  float _sampleTime = 1.0f;

  float _last_select_value = 0.f;

  kokopellivcv::dsp::LongPressButton _select_function_button;
  kokopellivcv::dsp::LongPressButton _select_mode_button;

  kokopellivcv::dsp::LongPressButton _fix_bounds_button;
  kokopellivcv::dsp::LongPressButton _forget_button;
  kokopellivcv::dsp::LongPressButton _loop_button;

  std::array<kokopellivcv::dsp::circle::Engine*, maxChannels> _engines;

  rack::dsp::ClockDivider _light_divider;
  rack::dsp::ClockDivider _button_divider;

  Options _options;

  Circle();

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

private:
  void processButtons();
  void processSelect();
};

} // namespace kokopellivcv

