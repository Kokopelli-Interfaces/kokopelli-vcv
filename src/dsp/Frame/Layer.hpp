#pragma once

#include "definitions.hpp"
#include "dsp/PhaseBuffer.hpp"

namespace myrisa {
namespace dsp {
namespace frame {

struct Layer {
  // TODO make me a vector for multiple signal recording
  PhaseBuffer *buffer;
  PhaseBuffer *manifestation_strength;

  int n_beats = 0;

  // int samples_per_beat = 0;

  Layer();
  // ~Layer();

  // void write(int division, float phase, float sample, float attenuation);
  // float getBufferPhase(int division, float phase);
  // float readSample(int division, float phase);
  // float readSampleWithAttenuation(int division, float phase, float attenuation);
  // float readSendAttenuation(int division, float phase);
};

} // namespace frame
} // namespace dsp
} // namespace myrisa
