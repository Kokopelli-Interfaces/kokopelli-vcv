#include "Layer.hpp"

#include <assert.h>
#include <math.h>

using namespace myrisa;

void Layer::addDivision() {
  vector<float> division_buffer(samples_per_division, 0.0f);
  division_buffers.push_back(division_buffer);

  vector<float> division_attenuation_send(samples_per_division, 0.0f);
  division_attenuation_sends.push_back(division_attenuation_send);

  divisions++;

  if (divisions > length) {
    length = divisions;
  }
}

void Layer::write(unsigned int current_division, float phase, float sample, float send_attenuation) {
  if (divisions == 0) {
    addDivision();
  }

  if (define_division_length) {
    division_buffers[0].push_back(sample);
    division_attenuation_sends[0].push_back(send_attenuation);
  } else {
    unsigned int division = current_division - start_division;
    if (divisions <= division) {
      addDivision();
    }

    // TODO what if phase == 1?
    float division_position = samples_per_division * phase;
    int sample_1 = floor(division_position);
    int sample_2 = ceil(division_position);
    if (sample_2 == samples_per_division) {
      sample_2 = 0;
    }

    float weight = division_position - sample_1;
    division_buffers[division][sample_1] += sample * (1 - weight);
    division_buffers[division][sample_2] += sample * weight;

    division_attenuation_sends[division][sample_1] += send_attenuation * (1 - weight);
    division_attenuation_sends[division][sample_2] += send_attenuation * weight;
  }
}


float Layer::readGeneric(unsigned int current_division, float phase, bool read_attenuation) {
  if (current_division < start_division) {
    return 0.0f;
  }

  int layer_division = (current_division - start_division) % length;
  if (layer_division < divisions) {
    int division_sample_i = floor(samples_per_division * phase);
    if (read_attenuation) {
      return division_attenuation_sends[layer_division][division_sample_i];
    } else {
      float sample = division_buffers[layer_division][division_sample_i];
      if (divisions > length && layer_division == 0) {
        sample += division_buffers[divisions-1][division_sample_i];
      }
      return sample;
    }
  }

  return 0.0f;
}

float Layer::readSample(unsigned int current_division, float phase) {
  if (fully_attenuated) {
      return 0.0f;
  }

  return readGeneric(current_division, phase, false);
}

float Layer::readAttenuation(unsigned int current_division, float phase) {
  return readGeneric(current_division, phase, true);
}

