#pragma once

#include "expanders.hpp"
#include "myrisa.hpp"

extern Model *modelSignal;
extern Model *modelFrame;

namespace myrisa {

struct SignalExpanderMessage : ExpanderMessage {
  float signal[MyrisaModule::maxChannels]{};
  int n_channels;
};

// TODO
struct FrameExpanderMessage : ExpanderMessage {
  float rate[MyrisaModule::maxChannels]{};
  float pos[MyrisaModule::maxChannels]{};
};

} // namespace myrisa
