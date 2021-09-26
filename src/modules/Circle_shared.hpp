#pragma once

#include "expanders.hpp"
#include "dsp/Signal.hpp"
#include "kokpelliinterfaces.hpp"

extern Model *modelSignal;
extern Model *modelCircle;

namespace kokpelliinterfaces {

struct SignalExpanderMessage : ExpanderMessage {
  float signal[KokpelliInterfacesModule::maxChannels]{};
  float sel_signal[KokpelliInterfacesModule::maxChannels]{};
  kokpelliinterfaces::dsp::SignalType signal_type;
  int n_channels;
};

} // namespace kokpelliinterfaces
