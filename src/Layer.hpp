#pragma once

#include <vector>

using namespace std;

namespace myrisa {

struct Layer {
  unsigned int start_division = 0;
  int divisions = 0;
  int length = 0;
  int samples_per_division = 0;
  int define_division_length = false;

  vector<vector<float>> division_buffers;
  vector<vector<float>> division_attenuation_sends;

  vector<Layer*> target_layers;

  bool fully_attenuated = false;
  bool attenuation_flag = false;

  void step(unsigned int phase, float in, float attenuation_power);
  void write(unsigned int division, float phase, float sample,
             float send_attenuation);
  void startRecording(vector<Layer*> selected_layers,
                      unsigned int phase, unsigned int division_length);

  void addDivision();
  void endRecording(unsigned int phase, unsigned int division_length);
  float readAttenuation(unsigned int current_division, float phase);
  float readSample(unsigned int current_division, float phase);
  float readGeneric(unsigned int current_division, float phase,
                    bool read_attenuation);
};

} // namespace myrisa
