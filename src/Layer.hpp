#pragma once

#include <math.h>
#include <vector>

#include "dsp/Interpolation.hpp"
#include "assert.hpp"

using namespace std;

namespace myrisa {

struct Layer {
private:
  int divisions = 0;

  vector<vector<float>> division_buffers;
  vector<vector<float>> division_attenuation_sends;

  bool attenuation_flag = false;

  void addDivision();
public:
  int define_division_length = false;
  int length = 0;
  int samples_per_division = 0;
  unsigned int start_division = 0;
  vector<Layer *> target_layers;
  bool fully_attenuated = false;

  void step(unsigned int phase, float in, float attenuation_power);
  void write(unsigned int division, float phase, float sample,
             float send_attenuation);
  float readAttenuation(unsigned int current_division, float phase, float sample_time);
  float readSample(unsigned int current_division, float phase, float sample_time);
};

} // namespace myrisa
