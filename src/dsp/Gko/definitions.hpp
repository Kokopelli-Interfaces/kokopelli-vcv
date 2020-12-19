#pragma once

#include <vector>

namespace myrisa {
namespace dsp {
namespace gko {

enum TimeFrame { TIMELINE, SELECTED_LAYERS, ACTIVE_LAYER };

struct TimePosition {
  unsigned int beat = 0;
  double phase = 0.f;
};

/**
  At each step, what the Engine does to it's Timeline is a function of these parameters.
  See the description of Record in the README for behaviour.
*/
struct RecordInterface {
  enum Mode {EXTEND, DUB, REPLACE};

  TimeFrame time_frame = TimeFrame::SELECTED_LAYERS;
  Mode mode = Mode::DUB;
  float strength = 0.f;

  float _recordActiveThreshold = 0.0001f;

  inline bool active() {
    return _recordActiveThreshold < strength;
  }
};

struct Connection {
  float to[MyrisaModule::maxChannels]{};
  float from[MyrisaModule::maxChannels]{};

  myrisa::dsp::SignalType signal_type = myrisa::dsp::SignalType::AUDIO;
  int send_channels = 1;
  std::string label = "";
  bool active = true;
};


struct Options {
  bool use_antipop = false;
};

} // namespace gko
} // namespace dsp
} // namespace myrisa
