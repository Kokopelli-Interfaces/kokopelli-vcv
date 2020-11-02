#pragma once

#include "myrisa.hpp"
#include "expanders.hpp"

extern Model *modelSignal;
extern Model *modelFrame;

namespace myrisa {

struct SignalExpanderMessage : ExpanderMessage {
  float signal[MyrisaModule::maxChannels]{};
};

} // namespace myrisa
