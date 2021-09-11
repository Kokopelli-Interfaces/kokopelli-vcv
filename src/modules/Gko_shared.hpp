#pragma once

#include "expanders.hpp"
#include "dsp/Signal.hpp"
#include "tribalinterfaces.hpp"

extern Model *modelSignal;
extern Model *modelGko;

namespace tribalinterfaces {

struct SignalExpanderMessage : ExpanderMessage {
  float signal[TribalInterfacesModule::maxChannels]{};
  float sel_signal[TribalInterfacesModule::maxChannels]{};
  tribalinterfaces::dsp::SignalType signal_type;
  int n_channels;
};

} // namespace tribalinterfaces
