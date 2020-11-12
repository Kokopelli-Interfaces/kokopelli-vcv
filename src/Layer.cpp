#include "Layer.hpp"

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
  ASSERT(0.0f, <=, phase);
  ASSERT(phase, <=, 1.0f);

  unsigned int division = current_division - start_division;
  if (divisions <= division) {
    addDivision();
  }

  if (divisions == 1 && define_division_length) {
    division_buffers[0].push_back(sample);
    division_attenuation_sends[0].push_back(send_attenuation);
    samples_per_division = division_buffers[0].size();
  } else {
    float division_position = samples_per_division * phase;
    int sample_1 = floor(division_position);
    int sample_2 = ceil(division_position);
    if (sample_2 == samples_per_division) {
      sample_2 = 0;
    }

    float weight = division_position - sample_1;

    ASSERT(division_buffers.size(), >, division);
    ASSERT(division_buffers[division].size(), >, sample_1);
    ASSERT(division_buffers[division].size(), >, sample_2);

    division_buffers[division][sample_1] += sample * (1 - weight);
    division_buffers[division][sample_2] += sample * weight;

    division_attenuation_sends[division][sample_1] += send_attenuation * (1 - weight);
    division_attenuation_sends[division][sample_2] += send_attenuation * weight;
  }
}

float getSampleAtPhase(vector<float> &buffer, double phase, Interpolations interpolation_mode) {
  switch (interpolation_mode) {
  case NONE:
    return buffer[floor(phase)];
    break;
  case LINEAR:
    return interpolateLinearD(buffer.data(), phase);
    break;
  case HERMITE:
    return InterpolateHermite(buffer.data(), phase,
                              buffer.size());
    break;
  case BSPLINE:
    return interpolateBSpline(buffer.data(), phase);
    break;
  default:
    return buffer[floor(phase)];
  }
}

float Layer::readGeneric(unsigned int current_division, float phase, bool read_attenuation, float sample_time) {
  if (current_division < start_division) {
    return 0.0f;
  }

  float sample = 0.0f;
  int layer_division = (current_division - start_division) % length;
  if (layer_division < divisions) {
    double division_phase = samples_per_division * phase;

    if (read_attenuation) {
      sample = getSampleAtPhase(division_attenuation_sends[layer_division], division_phase, Interpolations::LINEAR);
    } else {
      sample = getSampleAtPhase(division_buffers[layer_division], division_phase, Interpolations::HERMITE);
      if (divisions > length && layer_division == 0) {
        sample += getSampleAtPhase(division_buffers[divisions - 1], division_phase, Interpolations::HERMITE);
      }

      sample = antipop_filter.process(sample, sample_time);
    }
  }

  return sample;
}

float Layer::readSample(unsigned int current_division, float phase,
                        float sample_time) {
  if (fully_attenuated || current_division < start_division) {
      return 0.0f;
  }

  float sample = 0.0f;
  int layer_division = (current_division - start_division) % length;
  if (layer_division < divisions) {
    double division_phase = samples_per_division * phase;
    sample = getSampleAtPhase(division_buffers[layer_division],
                              division_phase, Interpolations::HERMITE);

    // add the trimmed ending division to the first division for smoother looping
    if (divisions > length && layer_division == 0) {
      sample += getSampleAtPhase(division_buffers[divisions - 1],
                                  division_phase, Interpolations::HERMITE);
    }

    sample = antipop_filter.process(sample, sample_time);
  }

  return sample;
}

float Layer::readAttenuation(unsigned int current_division, float phase, float sample_time) {
  if (current_division < start_division) {
    return 0.0f;
  }

  float sample = 0.0f;
  int layer_division = (current_division - start_division) % length;
  if (layer_division < divisions) {
    double division_phase = samples_per_division * phase;

    sample = getSampleAtPhase(division_attenuation_sends[layer_division],
                                division_phase, Interpolations::LINEAR);
  }

  return sample;
}

