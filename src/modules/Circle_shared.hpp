#pragma once

#include "expanders.hpp"
#include "dsp/Signal.hpp"
#include "kokopelliinterfaces.hpp"

extern Model *modelSignal;
extern Model *modelCircle;

namespace kokopelliinterfaces {

struct SignalExpanderMessage : ExpanderMessage {
  float signal[KokopelliInterfacesModule::maxChannels]{};
  float sel_signal[KokopelliInterfacesModule::maxChannels]{};
  kokopelliinterfaces::dsp::SignalType signal_type;
  int n_channels;
};

} // namespace kokopelliinterfaces
