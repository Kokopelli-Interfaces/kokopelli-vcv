#pragma once

#include "kokopellivcv.hpp"

extern Model *modelEther;

namespace kokopellivcv {

struct Ether : KokopelliVcvModule {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputId {
		NUM_OUTPUTS
	};
  enum LightIds {
    NUM_LIGHTS
	};

  float _sample_time = 1.0f;

  Ether();

  void sampleRateChange() override;
};

} // namespace kokopellivcv

