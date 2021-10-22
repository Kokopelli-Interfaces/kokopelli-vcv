#pragma once

#include "rack.hpp"
#include "dsp/misc/signal.hpp"
#include "LiveNode.hpp"
#include "EmernetNodeGroup.hpp"
#include "boost/variant.hpp"


#include <vector>

using Node = boost::variant<LiveNode*, EmernetNodeGroup*>;

namespace kokopellivcv {
namespace dsp {
namespace circle {

typedef std::vector<unsigned int> emernet_node_id;

union Node {
  LiveNode* member;
  EmernetNodeGroup* group;
};

enum class LoopMode { None, Group, Member };

struct Time {
  unsigned int beat = 0;
  double phase = 0.f;
};

struct State {
  bool loving = false;
  emernet_node_id focused_member_id;
  std::vector<emernet_node_id> active_nodes;
  LoopMode loop_mode = LoopMode::Group;
};

struct Inputs {
  float ext_phase = 0.f;
  float in = 0.f;
  float love = 0.f;

  static constexpr float _loveActiveThreshold = 0.0001f;
  static inline bool loveActive(float love) {
    return _loveActiveThreshold < love;
  }

  static inline float readIn(float in, float love) {
    // avoids pops when engaging / disengaging love parameter
    if (_loveActiveThreshold < love && love <= 0.033f) {
      float engage_attenuation = -1.f * pow((30.f * love - _loveActiveThreshold), 3) + 1.f;
      engage_attenuation = rack::clamp(engage_attenuation, 0.f, 1.f);
      return in * (1.f - engage_attenuation);
    }

    return in;
  }
};

struct Parameters {
  bool use_ext_phase = false;
  float sample_time = 1.0f;

  // TODO make me an array to support MIX4 & PLAY
  kokopellivcv::dsp::SignalType signal_type;
};

struct Options {
  bool use_antipop = false;
  bool strict_recording_lengths = true;
  bool bipolar_phase_input = false;
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
