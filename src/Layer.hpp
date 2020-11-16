#pragma once

#include <math.h>
#include <vector>

#include "rack.hpp"
#include "dsp/PhaseBuffer.hpp"

using namespace std;

namespace myrisa {

struct Layer {
private:
  PhaseBuffer *buffer;
  PhaseBuffer *send_attenuation;

  // relative to start_division
  pair<double, double> recording_offset_in_layer;

  bool inBounds(double scene_position) {
    if (scene_position < start_division + recording_offset_in_layer.first) {
      return false;
    }

    double layer_position = fmod(scene_position - start_division, length);
    return (recording_offset_in_layer.first <= layer_position && layer_position <= recording_offset_in_layer.second);
  }

  double getBufferPhase(double scene_position) {
    double recording_length = recording_offset_in_layer.second - recording_offset_in_layer.first;
    double layer_position = fmod(scene_position - start_division, length);
    return (layer_position - recording_offset_in_layer.first) / recording_length;
  }

public:
  Layer(double layer_start_division, double layer_length) {
    start_division = layer_start_division;
    length = layer_length;
    // TODO more buffers in future, according to frame IO modules
    // TODO free buffers ?
    buffer = new PhaseBuffer(PhaseBuffer::Type::AUDIO);
    send_attenuation = new PhaseBuffer(PhaseBuffer::Type::PARAM);
  }

  virtual ~Layer() {
    delete buffer;
    delete send_attenuation;
  }

  double start_division;
  double length;
  int num_samples = 0;

  vector<Layer *> target_layers;
  bool fully_attenuated = false;
  float sample_time = 1.0;

  void write(double scene_position, float sample, float attenuation) {
    // have to consider case where we are recording with external phase
    // e.g. one could start recording forward and then go in reverse
    double position_relative_to_start_division = scene_position - start_division;

    if (num_samples == 0) {
      recording_offset_in_layer.first = position_relative_to_start_division;
      recording_offset_in_layer.second = position_relative_to_start_division;
      buffer->addToBack(sample);
      send_attenuation->addToBack(attenuation);
      num_samples++;
    } else if (position_relative_to_start_division < recording_offset_in_layer.first) {
      buffer->addToFront(sample);
      send_attenuation->addToFront(attenuation);
      recording_offset_in_layer.first = position_relative_to_start_division;
      num_samples++;
    } else if (recording_offset_in_layer.second <= position_relative_to_start_division) {
      buffer->addToBack(sample);
      send_attenuation->addToBack(attenuation);
      recording_offset_in_layer.second = position_relative_to_start_division;
      num_samples++;
    } else {
      double buffer_phase = getBufferPhase(scene_position);
      printf("REPALCING: buf phase %f, pos_rel_start %f, recording_offsets: %f %f, lyaer len %f\n", buffer_phase, position_relative_to_start_division, recording_offset_in_layer.first, recording_offset_in_layer.second, length);
      buffer->replace(buffer_phase, sample);
      send_attenuation->replace(getBufferPhase(scene_position), attenuation);
    }
  }

  float readSample(double scene_position) {
    if (!inBounds(scene_position)) {
      return 0.0f;
    }
    return buffer->read(getBufferPhase(scene_position));
  }

  float readSampleWithAttenuation(double scene_position, double attenuation) {
    float sample = readSample(scene_position);
    if (sample == 0.0f) {
      return 0.0f;
    }
    return buffer->getAttenuatedSample(sample, attenuation);
  }

  float readSendAttenuation(double scene_position) {
    if (!inBounds(scene_position)) {
      return 0.0f;
    }

    return send_attenuation->read(getBufferPhase(scene_position));
  }
};

} // namespace myrisa
