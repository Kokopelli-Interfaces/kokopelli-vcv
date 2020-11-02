#pragma once

#include "myrisa.hpp"
#include "expanders.hpp"

extern Model *modelSignal;

namespace myrisa {

struct SignalExpanderMessage : ExpanderMessage {
  float signal[MyrisaModule::maxChannels]{};
};

} // namespace myrisa
