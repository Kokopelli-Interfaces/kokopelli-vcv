#include "Frame_IO.hpp"

namespace myrisa {

  struct Signal : ExpandableModule<SignalExpanderMessage, MyrisaModule> {
  enum ParamIds { MIX_PARAM, VCA_PARAM, NUM_PARAMS };
  enum InputIds { IN_INPUT, MIX_INPUT, VCA_INPUT, NUM_INPUTS };
  enum OutputIds { SEL_OUTPUT, OUT_OUTPUT, NUM_OUTPUTS };
  enum LightIds { NUM_LIGHTS };

  Signal() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(MIX_PARAM, 0.f, 1.f, 0.f, "Mix");
    configParam(VCA_PARAM, 0.f, 1.f, 0.f, "VCA");
  }

  SignalExpanderMessage _dummyExpanderMessage;

  void processAlways(const ProcessArgs& args) override;
};

} // namespace myrisa
