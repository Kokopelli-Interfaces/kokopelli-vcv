#pragma once

#include "definitions.hpp"
#include "PhaseBuffer.hpp"
#include "dsp/PhaseOscillator.hpp"
#include "dsp/Signal.hpp"

#include <vector>

namespace kokopelli {
namespace dsp {
namespace circle {

// A member which sings and also may lead a circle.
struct Member {
  unsigned int _n_beats = 0;
  unsigned int _beat = 0;

  std::vector<Member*> _circle;

  std::vector<float> _current_attenuation;
  std::vector<float> _last_calculated_attenuation;
  rack::dsp::ClockDivider _attenuation_calculator_divider;

  PhaseOscillator _phase_oscillator;
  PhaseAnalyzer _phase_analyzer;

  bool _alive = true;
  bool _loop = false;

  // TODO
  // std::vector<PhaseBuffer*> recordings;
  // std::vector<kokopelli::dsp::SignalType> types;

  PhaseBuffer *_in;
  PhaseBuffer *_love;

  inline Member(kokopelli::dsp::SignalType signal_type) {
    _in = new PhaseBuffer(signal_type);
    _love = new PhaseBuffer(kokopelli::dsp::SignalType::PARAM);
    _attenuation_calculator_divider.setDivision(2000);
  }

  inline ~Member() {
    delete _in;
    delete _love;
  }

  std::vector<Member*> getMembersFromIdx(std::vector<unsigned int> member_idx);
  void updateMemberAttenuations(unsigned int beat, float phase);

  float advance(Interface *interface);

  void prevBeat();
  void nextBeat();

  void listen(float in, float love);
  bool alive();
  float sing(float phase);
};

} // namespace circle
} // namespace dsp
} // namespace kokopelli
