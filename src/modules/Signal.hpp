#pragma once

#include <string>
#include "GkoInterface.hpp"
#include "module.hpp"
#include "menu.hpp"
#include "Gko.hpp"

extern Model *modelSignal;

namespace myrisa {

struct Signal : MyrisaModule {
  enum ParamIds { IN_ATTENUATION_PARAM, OUT_ATTENUATION_PARAM, NUM_PARAMS };
  enum InputIds { IN_INPUT, IN_ATTENUATION_INPUT, OUT_ATTENUATION_INPUT, NUM_INPUTS };
  enum OutputIds { SEL_OUTPUT, OUT_OUTPUT, NUM_OUTPUTS };
  enum LightIds { NUM_LIGHTS };

  struct Engine {
    float in_attenuation = 0.5f;
    float out_attenuation = 0.f;
  };

  Engine *_engines[maxChannels] {};

  GkoChannel* _gko_channel;

  myrisa::dsp::SignalType _signal_type = myrisa::dsp::SignalType::AUDIO;

  Signal() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(IN_ATTENUATION_PARAM, 0.f, 1.f, 0.f, "In Attenuation");
    configParam(OUT_ATTENUATION_PARAM, 0.f, 1.f, 0.f, "Out Attenuation");

    _gko_channel = new GkoChannel();
    _gko_channel->label = std::string("Sig%d", gko_channels.size()+1);
    gko_channels.push_back(_gko_channel);

    printf("add, gko ch's %ld\n", gko_channels.size());
  }

  ~Signal() {
    _gko_channel->active = false;
  }

  void modulateChannel(int c) override;
  void addChannel(int c) override;
  void removeChannel(int c) override;
  int channels() override;
  void processAlways(const ProcessArgs &args) override;
  void processChannel(const ProcessArgs &args, int channel) override;
};

} // namespace myrisa
