#pragma once

#include "Gko_shared.hpp"
#include "menu.hpp"
#include "dsp/Gko/Engine.hpp"
#include "dsp/LongPressButton.hpp"
#include "widgets.hpp"
#include <math.h>

namespace myrisa {

struct Gko : ExpanderModule<SignalExpanderMessage, MyrisaModule> {
  enum ParamIds {
    SELECT_PARAM,
    SELECT_MODE_PARAM,
    SELECT_FUNCTION_PARAM,
    SKIP_BACK_PARAM,
    RECORD_ON_OUTER_LOOP_PARAM,
    UNFIX_BOUNDS_PARAM,
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
    ENUMS(UNFIX_BOUNDS_LIGHT, 3),
    ENUMS(RECORD_ON_OUTER_LOOP_LIGHT, 3),
    ENUMS(PHASE_LIGHT, 3),
    NUM_LIGHTS
  };

  SignalExpanderMessage *_to_signal = nullptr;
  SignalExpanderMessage *_from_signal = nullptr;

  float _sampleTime = 1.0f;

  float _last_select_value = 0.f;

  myrisa::dsp::LongPressButton _select_function_button;
  myrisa::dsp::LongPressButton _select_mode_button;

  myrisa::dsp::LongPressButton _unfix_bounds_button;
  myrisa::dsp::LongPressButton _record_on_outer_loop_button;
  myrisa::dsp::LongPressButton _skip_back_button;

  std::array<myrisa::dsp::gko::Engine*, maxChannels> _engines;

  rack::dsp::ClockDivider _light_divider;
  rack::dsp::ClockDivider _button_divider;

  Options _options;

  Gko();

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

} // namespace myrisa

