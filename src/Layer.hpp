#pragma once

#include <math.h>
#include <vector>

#include "rack.hpp"
#include "dsp/PhaseBuffer.hpp"
#include "dsp/Interpolation.hpp"

using namespace std;

namespace myrisa {

struct Layer {
private:
  PhaseBuffer buffer;
  PhaseBuffer attenuation_send;
  // TODO maybe have this a part of a scene to avoid duplication
  rack::dsp::ClockDivider attenuation_write_divider;


  double rescalePhase(double scene_phase) {
    int scene_division = floor(scene_phase);
    double length = position.second - position.first;
    return (scene_phase - position.first) / length;
  }

public:
  Layer() {
    attenuation_write_divider.setDivision(256);
  }

  vector<Layer *> target_layers;
  pair<double, double> scene_position = NULL;

  bool fully_attenuated = false;

  bool inRange(double scene_phase) {
    int scene_division = floor(scene_phase);

  }

  void append(double phase, float sample, float attenuation) {
    buffer.append(sample);

    if (attenuation_write_divider.process()) {
      attenuation_send.append(attenuation);
    }
  }

  float replace(double phase, float sample, float attenuation) {
    ASSERT(position, !=, NULL);

    buffer.replace(rescalePhase(phase), sample);

    if (attenuation_write_divider.process()) {
      attenuation_send.replace(rescalePhase(phase), attenuation);
    }
  }

  float readSample(double phase) {
    ASSERT(position, !=, NULL);

    if (fully_attenuated || !inRange(phase)) {
      return 0.0f;
    }

    return buffer.read(rescalePhase(phase), Interpolations::HERMITE);
  }

  float readAttenuation(double phase) {
    ASSERT(position, !=, NULL);

    return send_attenuation.read(rescalePhase(phase), Interpolations::LINEAR);
  }
};

} // namespace myrisa
