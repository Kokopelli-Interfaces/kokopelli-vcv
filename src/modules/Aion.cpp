#include "Aion.hpp"
#include "AionWidget.hpp"

void Aion::processChannel(const ProcessArgs& args, int channel_i) {
  return;
}

Model* modelAion = rack::createModel<Aion, AionWidget>("KokopelliInterfaces-Aion");
