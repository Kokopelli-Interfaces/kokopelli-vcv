#pragma once

#include "Circle_shared.hpp"
#include "menu.hpp"
#include "dsp/Circle/Engine.hpp"
#include "dsp/LongPressButton.hpp"
#include "widgets.hpp"
#include <math.h>

namespace kokopelliinterfaces {

struct Circle : ExpanderModule<SignalExpanderMessage, KokopelliInterfacesModule> {
  enum ParamIds {
    SELECT_PARAM,
    SELECT_MODE_PARAM,
    SELECT_FUNCTION_PARAM,
    REFLECT_PARAM,
    NEXT_MEMBER_PARAM,
    PREV_MEMBER_PARAM,
    LOVE_PARAM,
    NUM_PARAMS
  };
  enum InputIds { SCENE_INPUT, RECORD_INPUT, PHASE_INPUT, NUM_INPUTS };
  enum OutputIds { PHASE_OUTPUT, NUM_OUTPUTS };
  enum LightIds {
    ENUMS(SELECT_FUNCTION_LIGHT, 3),
    ENUMS(SELECT_MODE_LIGHT, 3),
    ENUMS(REFLECT_LIGHT, 3),
    ENUMS(RECORD_LIGHT, 3),
    ENUMS(PREVIOUS_MEMBER_LIGHT, 3),
    ENUMS(NEXT_MEMBER_LIGHT, 3),
    ENUMS(PHASE_LIGHT, 3),
    NUM_LIGHTS
  };

  SignalExpanderMessage *_to_signal = nullptr;
  SignalExpanderMessage *_from_signal = nullptr;

  float _sampleTime = 1.0f;

  float _last_select_value = 0.f;

  kokopelliinterfaces::dsp::LongPressButton _select_function_button;
  kokopelliinterfaces::dsp::LongPressButton _select_mode_button;

  kokopelliinterfaces::dsp::LongPressButton _previous_member_button;
  kokopelliinterfaces::dsp::LongPressButton _next_member_button;
  kokopelliinterfaces::dsp::LongPressButton _reflect_button;

  std::array<kokopelliinterfaces::dsp::circle::Engine*, maxChannels> _engines;

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

} // namespace kokopelliinterfaces

