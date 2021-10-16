#pragma once

#include "rack.hpp"
#include "dsp/Signal.hpp"
#include "CircleMember.hpp"
#include "CircleGroup.hpp"

#include <vector>

namespace kokopellivcv {
namespace dsp {
namespace circle {

typedef std::vector<unsigned int> voice_id;

typedef std::variant<CircleMember*, CircleGroup*> CircleVoice;

enum class LoopMode { None, Group, Member };

struct TimePosition {
  unsigned int beat = 0;
  double phase = 0.f;
};

struct Parameters {
  bool use_ext_phase = false;
  float ext_phase = 0.f;
  float sample_time = 1.0f;

  float in = 0.f;
  float love = 0.f;

  // TODO make me an array to support MIX4 & PLAY
  kokopellivcv::dsp::SignalType signal_type;

  float _loveActiveThreshold = 0.0001f;

  inline bool loveActive() {
    return _loveActiveThreshold < love;
  }

  inline float readIn() {
    // avoids pops when engaging / disengaging love parameter
    if (_loveActiveThreshold < love && love <= 0.033f) {
      float engage_attenuation = -1.f * pow((30.f * love - _loveActiveThreshold), 3) + 1.f;
      engage_attenuation = rack::clamp(engage_attenuation, 0.f, 1.f);
      return in * (1.f - engage_attenuation);
    }

    return in;
  }
};

struct Options {
  bool use_antipop = false;
  bool strict_recording_lengths = true;
  bool bipolar_phase_input = false;
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
