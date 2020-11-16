#pragma once

#include <math.h>
#include <vector>

#include "rack.hpp"
#include "PhaseBuffer.hpp"

using namespace std;

namespace myrisa {

struct Layer {
private:
  PhaseBuffer *buffer;
  PhaseBuffer *send_attenuation;

  // relative to start_division
  pair<float, float> recording_offset_in_layer;

  bool inBounds(float scene_position) {
    if (scene_position < start_division + recording_offset_in_layer.first) {
      return false;
    }

    float layer_position = fmod(scene_position - start_division, length);
    return (recording_offset_in_layer.first <= layer_position && layer_position <= recording_offset_in_layer.second);
  }

  float getBufferPhase(float scene_position) {
    float recording_length = recording_offset_in_layer.second - recording_offset_in_layer.first;
    float layer_position = fmod(scene_position - start_division, length);
    ASSERT(0, <, recording_length);
    float buffer_phase = (layer_position - recording_offset_in_layer.first) / recording_length;
    return buffer_phase;
  }

public:
  Layer(float layer_start_division, float layer_length) {
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

  float start_division;
  float length;
  int num_samples = 0;

  vector<Layer *> target_layers;
  bool fully_attenuated = false;
  float sample_time = 1.0f;

  void writeByCreatingDivision(float sample, float attenuation) {
    if (num_samples == 0) {
      recording_offset_in_layer.first = 0.0f;
      recording_offset_in_layer.second = 1.0f;
    }

    buffer->addToBack(sample);
    send_attenuation->addToBack(attenuation);
    num_samples++;
  }

  void write(float scene_position, float sample, float attenuation) {
    // have to consider case where we are recording with external phase
    // e.g. one could start recording forward and then go in reverse
    float position_relative_to_start_division = scene_position - start_division;

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
    } else if (recording_offset_in_layer.second < position_relative_to_start_division) {
      buffer->addToBack(sample);
      send_attenuation->addToBack(attenuation);
      recording_offset_in_layer.second = position_relative_to_start_division;
      num_samples++;
      // printf("ADDTOBACK: pos_rel_start %f, recording_offsets: %f "
      //        "%f, lyaer len %f\n",
      //        position_relative_to_start_division,
      //        recording_offset_in_layer.first, recording_offset_in_layer.second,
      //        length);
    } else {
      float buffer_phase = getBufferPhase(scene_position);
      // printf("REPALCING: scene pos %f, buf phase %f, pos_rel_start %f, "
      //        "recording_offsets: %f %f, lyaer len %f\n",
      //        scene_position, buffer_phase, position_relative_to_start_division,
      //        recording_offset_in_layer.first, recording_offset_in_layer.second,
      //        length);
      buffer->replace(buffer_phase, sample);
      send_attenuation->replace(getBufferPhase(scene_position), attenuation);
    }
  }

  float readSample(float scene_position) {
    float sample = 0.0f;
    if (inBounds(scene_position)) {
      sample = buffer->read(getBufferPhase(scene_position));
    }

    return sample;
  }

  float readSampleWithAttenuation(float scene_position, float attenuation) {
    float sample = readSample(scene_position);
    if (sample == 0.0f) {
      return 0.0f;
    }
    return buffer->getAttenuatedSample(sample, attenuation);
  }

  float readSendAttenuation(float scene_position) {
    if (!inBounds(scene_position)) {
      return 0.0f;
    }

    return send_attenuation->read(getBufferPhase(scene_position));
  }
};

} // namespace myrisa
