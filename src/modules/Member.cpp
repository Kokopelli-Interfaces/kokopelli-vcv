#include "Member.hpp"
#include "MemberWidget.hpp"

Member::Member() {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
}

void Member::sampleRateChange() {
  _sample_time = APP->engine->getSampleTime();
}

Model* modelMember = rack::createModel<Member, MemberWidget>("KokopelliInterfaces-Member");
