#pragma once

#include "Gko_shared.hpp"
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
    TIME_FRAME_PARAM,
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
    ENUMS(TIME_FRAME_LIGHT, 3),
    ENUMS(RECORD_LIGHT, 3),
    ENUMS(RECORD_MODE_LIGHT, 3),
    ENUMS(RECORD_TIME_FRAME_LIGHT, 3),
    ENUMS(PHASE_LIGHT, 3),
    NUM_LIGHTS
  };

  SignalExpanderMessage *_to_signal = nullptr;
  SignalExpanderMessage *_from_signal = nullptr;

  float _sampleTime = 1.0f;

  myrisa::dsp::LongPressButton _record_mode_button;
  myrisa::dsp::LongPressButton _record_time_frame_button;
  myrisa::dsp::LongPressButton _time_frame_button;

  std::array<myrisa::dsp::gko::Engine*, maxChannels> _engines;
  rack::dsp::ClockDivider _light_divider;
  rack::dsp::ClockDivider _button_divider;

  Gko();

  void sampleRateChange() override;
  int channels() override;
  void modulateChannel(int c) override;
  void addChannel(int c) override;
  void removeChannel(int c) override;
  void processAlways(const ProcessArgs &args) override;
  void processChannel(const ProcessArgs &args, int channel) override;
  void postProcessAlways(const ProcessArgs &args) override;
  void updateLights(const ProcessArgs &args);

private:
  void processButtons();
};

} // namespace myrisa

