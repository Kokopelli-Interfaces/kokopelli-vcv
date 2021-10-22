#pragma once

#include "expanders.hpp"
#include "dsp/misc/signal.hpp"
#include "kokopellivcv.hpp"

extern Model *modelSignal;
extern Model *modelCircle;

namespace kokopellivcv {

struct SignalExpanderMessage : ExpanderMessage {
  float signal[KokopelliVcvModule::maxChannels]{};
  float sel_signal[KokopelliVcvModule::maxChannels]{};
  kokopellivcv::dsp::SignalType signal_type;
  int n_channels;
};

} // namespace kokopellivcv
