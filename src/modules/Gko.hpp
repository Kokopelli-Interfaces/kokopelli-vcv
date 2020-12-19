#pragma once

#include "GkoInterface.hpp"
#include "rack.hpp"
#include "module.hpp"
#include "myrisa.hpp"
#include "menu.hpp"
#include "dsp/Gko/Engine.hpp"
#include "dsp/LongPressButton.hpp"
#include "widgets.hpp"
#include <math.h>
#include <map>

extern Model *modelGko;

namespace myrisa {

struct Gko : MyrisaModule {
  enum ParamIds {
    SELECT_PARAM,
    SELECT_MODE_PARAM,
    SELECT_FUNCTION_PARAM,
    READ_TIME_FRAME_PARAM,
    RECORD_TIME_FRAME_PARAM,
    RECORD_MODE_PARAM,
    RECORD_PARAM,
    NUM_PARAMS
  };
  enum InputIds { SCENE_INPUT, RECORD_INPUT, PHASE_INPUT, NUM_INPUTS };
  enum OutputIds { PHASE_OUTPUT, NUM_OUTPUTS };
  enum LightIds {
    ENUMS(SELECT_FUNCTION_LIGHT, 3),
    ENUMS(SELECT_MODE_LIGHT, 3),
    ENUMS(READ_TIME_FRAME_LIGHT, 3),
    ENUMS(RECORD_LIGHT, 3),
    ENUMS(RECORD_MODE_LIGHT, 3),
    ENUMS(RECORD_TIME_FRAME_LIGHT, 3),
    ENUMS(PHASE_LIGHT, 3),
    NUM_LIGHTS
  };

  float _sampleTime = 1.0f;

  float _last_select_value = 0.f;

  std::vector<unsigned int> _saved_selected_layers_idx;

  myrisa::dsp::LongPressButton _select_function_button;
  myrisa::dsp::LongPressButton _select_mode_button;

  myrisa::dsp::LongPressButton _record_mode_button;
  myrisa::dsp::LongPressButton _record_time_frame_button;
  myrisa::dsp::LongPressButton _read_time_frame_button;

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
  bool hasConnections();
  void processSelect();
};

} // namespace myrisa

