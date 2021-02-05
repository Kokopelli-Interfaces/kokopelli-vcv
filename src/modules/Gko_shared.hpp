#pragma once

#include "expanders.hpp"
#include "dsp/Signal.hpp"
#include "myrisa.hpp"

extern Model *modelSignal;
extern Model *modelGko;

namespace myrisa {

struct SignalExpanderMessage : ExpanderMessage {
  float signal[MyrisaModule::maxChannels]{};
  float sel_signal[MyrisaModule::maxChannels]{};
  myrisa::dsp::SignalType signal_type;
  int n_channels;
};

} // namespace myrisa
