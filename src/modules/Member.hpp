#pragma once

#include "kokopellivcv.hpp"

extern Model *modelMember;

namespace kokopellivcv {

struct Member : KokopelliVcvModule {
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

  Member();

  void sampleRateChange() override;
};

} // namespace kokopellivcv

