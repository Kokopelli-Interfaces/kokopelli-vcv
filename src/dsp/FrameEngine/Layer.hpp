#pragma once

#include <math.h>
#include <vector>

#include "dsp/PhaseBuffer.hpp"
#include "modules/Frame_shared.hpp"
#include "rack.hpp"

using namespace std;
using namespace myrisa::util;

namespace myrisa {
namespace dsp {

class Layer {
private:
  PhaseBuffer *buffer;
  PhaseBuffer *send_attenuation;

public:
  virtual ~Layer() {
    delete buffer;
    delete send_attenuation;
  }

  RecordMode mode;
  int start_division = 0;
  int n_divisions = 0;
  int samples_per_division = 0;

  vector<Layer*> target_layers;
  bool fully_attenuated = false;

  Layer(RecordMode record_mode, int division, vector<Layer *> selected_layers,
        int layer_samples_per_division);

  void write(int division, float phase, float sample, float attenuation);
  float getBufferPhase(int division, float phase);
  float readSample(int division, float phase);
  float readSampleWithAttenuation(int division, float phase, float attenuation);
  float readSendAttenuation(int division, float phase);
};

} // namespace dsp
} // namespace myrisa
