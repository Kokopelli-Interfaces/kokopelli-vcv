#include "Engine.hpp"

using namespace myrisa::dsp::frame;

Layer::Layer(Delta::Mode record_mode, int division, vector<Layer*> selected_layers, int layer_samples_per_division, bool phase_defined) {
  buffer = new PhaseBuffer(PhaseBuffer::Type::AUDIO);
  send_attenuation = new PhaseBuffer(PhaseBuffer::Type::PARAM);
  samples_per_division = layer_samples_per_division;

  start_division = division;
  _mode = record_mode;
  // TODO remove me
  _phase_defined = phase_defined;

  if (!phase_defined) {
    n_divisions = 1;
  } else {
    for (auto selected_layer : selected_layers) {
      if (selected_layer && !selected_layer->fully_attenuated) {
        target_layers.push_back(selected_layer);
      }
    }

    if (_mode == Delta::Mode::DUB) {
      if (0 < selected_layers.size()) {
        auto most_recent_target_layer = selected_layers.back();
        n_divisions = most_recent_target_layer->n_divisions;
      } else {
        n_divisions = 1;
      }
    } else if (_mode == Delta::Mode::EXTEND) {
      n_divisions = 0;
    }
  }
}

Layer::~Layer() {
  delete buffer;
  delete send_attenuation;
}

void Layer::write(int division, float phase, float sample, float attenuation) {
  // TODO have to consider case where we are recording with external phase
  // e.g. one could start recording forward and then go in reverse
  assert(0.0 <= phase);
  assert(phase <= 1.0);
  assert(start_division <= division);

  if (!_phase_defined) {
    buffer->pushBack(sample);
    send_attenuation->pushBack(sample);
    samples_per_division++;
  } else {
    assert(0 < samples_per_division);

    if (_mode == Delta::Mode::DUB) {
      assert(n_divisions != 0);

      if (buffer->size() == 0) {
        buffer->resize(n_divisions * samples_per_division);
        send_attenuation->resize(n_divisions * samples_per_division);
      }

      float buffer_phase = getBufferPhase(division, phase);
      buffer->write(buffer_phase, sample);
      send_attenuation->write(buffer_phase, attenuation);

    } else if (_mode == Delta::Mode::EXTEND) {
      while (start_division + n_divisions <= division) {
        buffer->resize(buffer->size() + samples_per_division);
        send_attenuation->resize(send_attenuation->size() + samples_per_division);
        n_divisions++;
      }

      float buffer_phase = getBufferPhase(division, phase);
      buffer->write(buffer_phase, sample);
      send_attenuation->write(buffer_phase, attenuation);

    }
  }
}

float Layer::getBufferPhase(int division, float phase) {
  assert(0 <= n_divisions);
  int layer_division = (division - start_division) % n_divisions;
  float buffer_phase = (layer_division + phase) / n_divisions;
  return buffer_phase;
}

float Layer::readSample(int division, float phase) {
  if (division < start_division) {
    return 0.0f;
  }

  return buffer->read(getBufferPhase(division, phase));
}

float Layer::readSampleWithAttenuation(int division, float phase,
                                       float attenuation) {
  float sample = readSample(division, phase);
  if (sample == 0.0f) {
    return 0.0f;
  }
  return buffer->getAttenuatedSample(sample, attenuation);
}

float Layer::readSendAttenuation(int division, float phase) {
  if (division < start_division) {
    return 0.0f;
  }
  return send_attenuation->read(getBufferPhase(division, phase));
}
