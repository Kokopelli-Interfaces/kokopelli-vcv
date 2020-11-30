#include "Engine.hpp"

using namespace myrisa::dsp::frame;

Layer::Layer() {
  buffer = new PhaseBuffer(PhaseBuffer::Type::AUDIO);
  manifestation_strength = new PhaseBuffer(PhaseBuffer::Type::PARAM);

  // TODO remove me
  _phase_defined = phase_defined;

  if (!phase_defined) {
    n_beats = 1;
  } else {
    for (auto selected_layer : selected_layers) {
      if (selected_layer && !selected_layer->fully_attenuated) {
        target_layers.push_back(selected_layer);
      }
    }

    if (_mode == ManifestParams::Mode::DUB) {
      if (0 < selected_layers.size()) {
        auto most_recent_target_layer = selected_layers.back();
        n_beats = most_recent_target_layer->n_beats;
      } else {
        n_beats = 1;
      }
    } else if (_mode == ManifestParams::Mode::EXTEND) {
      n_beats = 0;
    }
  }
}

Layer::~Layer() {
  delete buffer;
  delete manifestation_strength;
}

void Layer::write(int beat, float phase, float sample, float attenuation) {
  // TODO have to consider case where we are recording with external phase
  // e.g. one could start recording forward and then go in reverse
  assert(0.0 <= phase);
  assert(phase <= 1.0);
  assert(start_beat <= beat);

  if (!_phase_defined) {
    buffer->pushBack(sample);
    manifestation_strength->pushBack(sample);
    samples_per_beat++;
  } else {
    assert(0 < samples_per_beat);

    if (_mode == ManifestParams::Mode::DUB) {
      assert(n_beats != 0);

      if (buffer->size() == 0) {
        buffer->resize(n_beats * samples_per_beat);
        manifestation_strength->resize(n_beats * samples_per_beat);
      }

      float buffer_phase = getBufferPhase(beat, phase);
      buffer->write(buffer_phase, sample);
      manifestation_strength->write(buffer_phase, attenuation);

    } else if (_mode == ManifestParams::Mode::EXTEND) {
      while (start_beat + n_beats <= beat) {
        buffer->resize(buffer->size() + samples_per_beat);
        manifestation_strength->resize(manifestation_strength->size() + samples_per_beat);
        n_beats++;
      }

      float buffer_phase = getBufferPhase(beat, phase);
      buffer->write(buffer_phase, sample);
      manifestation_strength->write(buffer_phase, attenuation);

    }
  }
}

float Layer::getBufferPhase(int beat, float phase) {
  assert(0 <= n_beats);
  int layer_beat = (beat - start_beat) % n_beats;
  float buffer_phase = (layer_beat + phase) / n_beats;
  return buffer_phase;
}

float Layer::readSample(int beat, float phase) {
  if (beat < start_beat) {
    return 0.f;
  }

  return buffer->read(getBufferPhase(beat, phase));
}

float Layer::readSampleWithAttenuation(int beat, float phase,
                                       float attenuation) {
  float sample = readSample(beat, phase);
  if (sample == 0.f) {
    return 0.f;
  }
  return buffer->getAttenuatedSample(sample, attenuation);
}

float Layer::readSendAttenuation(int beat, float phase) {
  if (beat < start_beat) {
    return 0.f;
  }
  return manifestation_strength->read(getBufferPhase(beat, phase));
}
