#pragma once

#include "expanders.hpp"
#include "dsp/Signal.hpp"
#include "kokopelli.hpp"

extern Model *modelSignal;
extern Model *modelCircle;

namespace kokopelli {

struct SignalExpanderMessage : ExpanderMessage {
  float signal[KokopelliModule::maxChannels]{};
  float sel_signal[KokopelliModule::maxChannels]{};
  kokopelli::dsp::SignalType signal_type;
  int n_channels;
};

} // namespace kokopelli
