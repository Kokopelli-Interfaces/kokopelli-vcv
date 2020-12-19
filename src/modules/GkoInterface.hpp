#pragma once

#include "dsp/Signal.hpp"
#include "module.hpp"
#include <string>

namespace myrisa {

struct GkoChannel {
  float to[MyrisaModule::maxChannels]{};
  float from[MyrisaModule::maxChannels]{};
  myrisa::dsp::SignalType signal_type = myrisa::dsp::SignalType::AUDIO;
  int send_channels = 1;
  std::string label = "";
  bool active = true;
};

extern std::vector<GkoChannel*> gko_channels;

} // namespace myrisa
