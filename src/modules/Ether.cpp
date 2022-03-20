#include "Ether.hpp"
#include "EtherWidget.hpp"

Ether::Ether() {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
}

void Ether::sampleRateChange() {
  _sample_time = APP->engine->getSampleTime();
}

Model* modelEther = rack::createModel<Ether, EtherWidget>("KokopelliInterfaces-Ether");
