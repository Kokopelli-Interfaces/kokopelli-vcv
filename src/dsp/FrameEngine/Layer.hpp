#pragma once

#include <math.h>
#include <vector>

#include "Section.hpp"
#include "dsp/PhaseBuffer.hpp"
#include "modules/Frame_shared.hpp"
#include "rack.hpp"

using namespace std;
using namespace myrisa::util;

namespace myrisa {
namespace dsp {

struct Layer {
private:
  PhaseBuffer *buffer;
  PhaseBuffer *send_attenuation;

public:
  Layer(RecordMode record_mode, int division, vector<Layer*> selected_layers, int layer_samples_per_division) {
    ASSERT(record_mode, !=, RecordMode::READ);

    buffer = new PhaseBuffer(PhaseBuffer::Type::AUDIO);
    send_attenuation = new PhaseBuffer(PhaseBuffer::Type::PARAM);
    samples_per_division = layer_samples_per_division;

    start_division = division;
    mode = record_mode;

    if (mode == RecordMode::DEFINE_DIVISION_LENGTH) {
      n_divisions = 1;
    } else {
      for (auto selected_layer : selected_layers) {
        if (selected_layer && !selected_layer->fully_attenuated) {
          target_layers.push_back(selected_layer);
        }
      }

      if (mode == RecordMode::DUB) {
        if (0 < selected_layers.size()) {
          auto most_recent_target_layer = selected_layers.back();
          n_divisions = most_recent_target_layer->n_divisions;
        } else {
          n_divisions = 1;
        }
      } else if (mode == RecordMode::EXTEND) {
        n_divisions = 0;
      }
    }
  }

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

  inline void write(int division, float phase, float sample, float attenuation) {
    // TODO have to consider case where we are recording with external phase
    // e.g. one could start recording forward and then go in reverse
    ASSERT(0.0, <=, phase);
    ASSERT(phase, <=, 1.0);
    ASSERT(start_division, <=, division);

    if (mode == RecordMode::DEFINE_DIVISION_LENGTH) {
      // ASSERT(division, ==, 0);
      buffer->pushBack(sample);
      send_attenuation->pushBack(sample);
      samples_per_division++;
    } else {
      ASSERT(0, <, samples_per_division);

      if (mode == RecordMode::DUB) {
        ASSERT(n_divisions, !=, 0);

        if (buffer->size() == 0) {
          buffer->resize(n_divisions * samples_per_division);
          send_attenuation->resize(n_divisions * samples_per_division);
        }

        float buffer_phase = getBufferPhase(division, phase);
        buffer->write(buffer_phase, sample);
        send_attenuation->write(buffer_phase, attenuation);

      } else if (mode == RecordMode::EXTEND) {
        while (start_division + n_divisions <= division) {
          buffer->resize(buffer->size() + samples_per_division);
          send_attenuation->resize(send_attenuation->size() + samples_per_division);
          n_divisions++;
        }

        float buffer_phase = getBufferPhase(division, phase);
        buffer->write(buffer_phase, sample);
        send_attenuation->write(buffer_phase, attenuation);

      } else if (mode == RecordMode::READ) {
        printf("Myrisa Frame: Write in read mode?? Frame is broken.\n");
        return;
      }
    }
  }

  inline float getBufferPhase(int division, float phase) {
    ASSERT(0, <=, n_divisions);
    int layer_division = (division - start_division) % n_divisions;
    float buffer_phase = (layer_division + phase) / n_divisions;
    return buffer_phase;
  }

  inline float readSample(int division, float phase) {
    if (division < start_division) {
      return 0.0f;
    }

    return buffer->read(getBufferPhase(division, phase));
  }

  inline float readSampleWithAttenuation(int division, float phase, float attenuation) {
    float sample = readSample(division, phase);
    if (sample == 0.0f) {
      return 0.0f;
    }
    return buffer->getAttenuatedSample(sample, attenuation);
  }

  inline float readSendAttenuation(int division, float phase) {
    if (division < start_division) {
      return 0.0f;
    }
    return send_attenuation->read(getBufferPhase(division, phase));
  }
};

} // namespace dsp
} // namespace myrisa
