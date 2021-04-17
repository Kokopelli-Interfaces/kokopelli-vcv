#pragma once

#include "definitions.hpp"
#include "Recording.hpp"
#include "dsp/Signal.hpp"

namespace myrisa {
namespace dsp {
namespace gko {

struct Layer {
  TimePosition _start;
  unsigned int _n_beats = 0;
  bool _loop = false;
  float start_offset = 0.f;

  // TODO
  // std::vector<Recording*> recordings;
  // std::vector<myrisa::dsp::SignalType> types;

  Recording *_in;
  Recording *_recording_strength;

  // FIXME change me to be an array of bools for O(1) lookup
  std::vector<unsigned int> target_layers_idx;

  inline Layer(TimePosition start, unsigned int n_beats, std::vector<unsigned int> target_layers_idx, myrisa::dsp::SignalType signal_type, int samples_per_beat) {
    this->_start = start;
    this->_n_beats = n_beats;
    this->target_layers_idx = target_layers_idx;

    _in = new Recording(signal_type, samples_per_beat);
    _recording_strength = new Recording(myrisa::dsp::SignalType::PARAM, samples_per_beat);
  }

  inline ~Layer() {
    delete _in;
    delete _recording_strength;
  }

  inline TimePosition getLayerPosition(TimePosition timeline_position) {
    TimePosition layer_position = timeline_position;
    layer_position.phase -= _start.phase;
    if (layer_position.phase < 0) {
      layer_position.beat -= 1;
      layer_position.phase += 1.0f;
    }

    layer_position.beat = layer_position.beat - _start.beat;
    if (_loop && 0 < layer_position.beat) {
      layer_position.beat = layer_position.beat % _n_beats;
    }

    return layer_position;
  }

  inline unsigned int getLayerBeat(TimePosition timeline_position) {
    return getLayerPosition(timeline_position).beat;
  }

  inline bool definedAtPosition(TimePosition timeline_position) {
    int beat = (int)timeline_position.beat - (int)_start.beat;
    if (beat < 0 || (beat == 0 && timeline_position.phase < _start.phase)) {
      return false;
    }

    if (!_loop && ((int)_n_beats < beat || ((int)_n_beats == (int)beat && timeline_position.phase < _start.phase))) {
      return false;
    }

    return true;
  }

  inline float readSignal(TimePosition timeline_position) {
    if (!definedAtPosition(timeline_position)) {
      return 0.f;
    }

    return _in->read(getLayerPosition(timeline_position));
  }

  inline float readRecordingStrength(TimePosition timeline_position) {
    if (!definedAtPosition(timeline_position)) {
      return 0.f;
    }

    return _recording_strength->read(getLayerPosition(timeline_position));
  }

  // TODO FIXME allow reverse recording
  inline bool writableAtPosition(TimePosition timeline_position) {
    int layer_beat = getLayerBeat(timeline_position);
    return (0 <= layer_beat && layer_beat < (int)_n_beats);
  }

  inline void write(TimePosition timeline_position, float in, float strength, bool phase_defined) {
    assert(_in->_buffer.size() <= _n_beats);
    assert(_recording_strength->_buffer.size() <= _n_beats);

    if (!phase_defined) {
      _in->pushBack(in);
      _recording_strength->pushBack(strength);
    } else if (writableAtPosition(timeline_position)) {
      _in->write(getLayerPosition(timeline_position), in);
   _recording_strength->write(getLayerPosition(timeline_position), strength);
    }
  }
};

} // namespace gko
} // namespace dsp
} // namespace myrisa
