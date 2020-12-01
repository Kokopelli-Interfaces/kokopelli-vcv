#pragma once

#include "definitions.hpp"
#include "Recording.hpp"

namespace myrisa {
namespace dsp {
namespace gko {

/**
  A Layer is one row in the Posline.
*/
struct Layer {
  float start = 0;
  float length = 0;
  bool loop = false;

  // TODO
  // std::vector<Recording*> recordings;
  // std::vector<Recording::Type> types;

  Recording *signal;
  Recording *manifestation_strength;
  std::vector<int> target_layers_idx;

  // TODO optinoal samples_per_beat?
  inline Layer(float start, float length, std::vector<int> target_layers_idx) {
    this->start = start;
    this->length = length;
    this->target_layers_idx = target_layers_idx;

    signal = new Recording(Recording::Type::AUDIO);
    manifestation_strength = new Recording(Recording::Type::PARAM);
  }

  inline ~Layer() {
    delete signal;
    delete manifestation_strength;
  }

  inline void pushBack(float signal_sample, float manifestation_strength_sample) {
    signal->pushBack(signal_sample);
    manifestation_strength->pushBack(manifestation_strength_sample);
  }

  inline bool activeAtTime(float time) {
    return loop ?
      start <= time && time <= start + length :
      start <= time;
  }

  inline float timeToRecordingPhase(float time) {
    assert(activeAtTime(time));
    float time_in_recording = std::fmod(time - start, length);
    return time_in_recording / length;
  }

  inline float readSignal(float time) {
    if (!activeAtTime(time)) {
      return 0.f;
    }

    return signal->read(timeToRecordingPhase(time));
  }


  inline float readManifestationStrength(float time) {
    if (!activeAtTime(time)) {
      return 0.f;
    }

    return manifestation_strength->read(timeToRecordingPhase(time));
  }

  inline void write(float time, float signal_sample, float manifestation_strength_sample) {
    assert(start <= time && time <= start + length);
    assert(0 < signal->size());
    assert(0 < manifestation_strength->size());

    signal->write(timeToRecordingPhase(time), signal_sample);
    manifestation_strength->write(timeToRecordingPhase(time), manifestation_strength_sample);
  }
};

} // namespace frame
} // namespace dsp
} // namespace myrisa
