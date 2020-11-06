#pragma once

#include "myrisa.hpp"
#include "expanders.hpp"

extern Model *modelSignal;
extern Model *modelFrame;

namespace myrisa {

struct SignalExpanderMessage : ExpanderMessage {
  float signal[MyrisaModule::maxChannels]{};
};

// TODO
struct FrameExpanderMessage : ExpanderMessage {
  float rate[MyrisaModule::maxChannels]{};
  float pos[MyrisaModule::maxChannels]{};
};

} // namespace myrisa
