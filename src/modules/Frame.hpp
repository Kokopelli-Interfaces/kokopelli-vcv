#pragma once

#include "Frame_shared.hpp"
#include "dsp/Frame/Engine.hpp"
#include "widgets.hpp"
#include "util/gui.hpp"
#include <math.h>

namespace myrisa {

struct Frame : ExpanderModule<SignalExpanderMessage, MyrisaModule> {
  enum ParamIds {
    LAYER_PARAM,
    SELECT_PARAM,
    SCENE_PARAM,
    MODE_SWITCH_PARAM,
    LOOP_PARAM,
    RECORD_MODE_PARAM,
    RECORD_CONTEXT_PARAM,
    DELTA_PARAM,
    NUM_PARAMS
  };
  enum InputIds { SCENE_INPUT, DELTA_INPUT, CLK_INPUT, NUM_INPUTS };
  enum OutputIds { PHASE_OUTPUT, NUM_OUTPUTS };
  enum LightIds {
    ENUMS(SELECT_LIGHT, 3),
    ENUMS(MODE_SWITCH_LIGHT, 3),
    ENUMS(LOOP_LIGHT, 3),
    ENUMS(STRUCTURE_LIGHT, 3),
    ENUMS(RECORD_MODE_LIGHT, 3),
    ENUMS(RECORD_CONTEXT_LIGHT, 3),
    ENUMS(PHASE_LIGHT, 3),
    NUM_LIGHTS
  };

  SignalExpanderMessage *_to_signal = nullptr;
  SignalExpanderMessage *_from_signal = nullptr;

  const float _recordThreshold = 0.05f;
  float _sampleTime = 1.0f;

  std::array<myrisa::dsp::frame::Engine*, maxChannels> _engines;
  rack::dsp::ClockDivider light_divider;

  Frame();

  void sampleRateChange() override;
  int channels() override;
  void modulateChannel(int c) override;
  void addChannel(int c) override;
  void removeChannel(int c) override;
  void processAlways(const ProcessArgs &args) override;
  void processChannel(const ProcessArgs &args, int channel) override;
  void postProcessAlways(const ProcessArgs &args) override;
  void updateLights(const ProcessArgs &args);
};

} // namespace myrisa

