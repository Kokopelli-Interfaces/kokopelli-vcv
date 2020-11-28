#pragma once

#include "definitions.hpp"
#include "dsp/PhaseBuffer.hpp"

namespace myrisa {
namespace dsp {
namespace frame {

class Layer {
private:
  PhaseBuffer *buffer;
  PhaseBuffer *send_attenuation;

public:
  Delta::Mode _mode;
  int start_division = 0;
  int n_divisions = 0;
  int samples_per_division = 0;

  // TODO
  bool _phase_defined = true;

  std::vector<Layer *> target_layers;
  bool fully_attenuated = false;

  Layer(Delta::Mode record_mode, int division, vector<Layer *> selected_layers,
        int layer_samples_per_division, bool phase_defined);
  ~Layer();

  void write(int division, float phase, float sample, float attenuation);
  float getBufferPhase(int division, float phase);
  float readSample(int division, float phase);
  float readSampleWithAttenuation(int division, float phase, float attenuation);
  float readSendAttenuation(int division, float phase);
};

} // namespace frame
} // namespace dsp
} // namespace myrisa
