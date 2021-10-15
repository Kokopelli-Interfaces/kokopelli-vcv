#include "kokopellivcv.hpp"

extern Model *modelSignal4;

namespace kokopellivcv {

struct Signal4 : KokopelliVcvModule {
  enum ParamIds { OUT_ATTENUATION_PARAM, IN_ATTENUATION_PARAM, NUM_PARAMS };
  enum InputIds {
    IN_1_INPUT,
    IN_2_INPUT,
    IN_3_INPUT,
    IN_4_INPUT,
    OUT_ATTENUATION_INPUT,
    IN_ATTENUATION_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    SEL_1_OUTPUT,
    SEL_2_OUTPUT,
    SEL_3_OUTPUT,
    SEL_4_OUTPUT,
    OUT_1_OUTPUT,
    OUT_2_OUTPUT,
    OUT_3_OUTPUT,
    OUT_4_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds { NUM_LIGHTS };

  Signal4() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(OUT_ATTENUATION_PARAM, 0.f, 1.f, 0.f, "Out Attenuation");
    configParam(IN_ATTENUATION_PARAM, 0.f, 1.f, 0.f, "Mix");
  }

  void processAlways(const ProcessArgs& args) override;
};

} // namespace kokopellivcv
