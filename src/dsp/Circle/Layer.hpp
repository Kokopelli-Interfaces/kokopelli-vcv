#pragma once

#include "definitions.hpp"
#include "Recording.hpp"
#include "dsp/Signal.hpp"

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct Layer {
  unsigned int _start_beat = 0;
  unsigned int _n_beats = 0;
  bool _loop = false;

  // TODO
  // std::vector<Recording*> recordings;
  // std::vector<kokopellivcv::dsp::SignalType> types;

  Recording *_in;
  Recording *_love;

  // FIXME change me to be an array of bools for O(1) lookup
  std::vector<unsigned int> target_layers_idx;

  inline Layer(unsigned int start_beat, unsigned int n_beats, std::vector<unsigned int> target_layers_idx, kokopellivcv::dsp::SignalType signal_type, int samples_per_beat) {
    this->_start_beat = start_beat;
    this->_n_beats = n_beats;
    this->target_layers_idx = target_layers_idx;

    _in = new Recording(signal_type, samples_per_beat);
    _love = new Recording(kokopellivcv::dsp::SignalType::PARAM, samples_per_beat);
  }

  inline ~Layer() {
    delete _in;
    delete _love;
  }

  inline bool isLooping() {
    return _loop;
  }
 
  inline void setLoop(bool loop) {
    _loop = loop;
  }

  inline unsigned int getLayerBeat(unsigned int timeline_beat) {
    int beat = timeline_beat - _start_beat;
    if (_loop && 0 < beat) {
      return beat % _n_beats;
    }

    return beat;
  }

  inline bool readableAtPosition(TimePosition timeline_position) {
    int layer_beat = getLayerBeat(timeline_position.beat);
    return (0 <= layer_beat && layer_beat < (int)_n_beats);
  }

  // TODO FIXME allow reverse recording
  inline bool writableAtPosition(TimePosition timeline_position) {
    return _start_beat <= timeline_position.beat && timeline_position.beat <= _start_beat + _n_beats;
  }

  inline TimePosition getRecordingPosition(TimePosition timeline_position) {
    TimePosition recording_position = timeline_position;
    recording_position.beat = getLayerBeat(timeline_position.beat);
    return recording_position;
  }

  inline float readSignal(TimePosition timeline_position) {
    if (!readableAtPosition(timeline_position)) {
      return 0.f;
    }

    return _in->read(getRecordingPosition(timeline_position));
  }

  inline float readRecordingLove(TimePosition timeline_position) {
    if (!readableAtPosition(timeline_position)) {
      return 0.f;
    }

    return _love->read(getRecordingPosition(timeline_position));
  }

  inline void write(TimePosition timeline_position, float in, float new_love, bool phase_defined) {
    assert(_in->_buffer.size() <= _n_beats);
    assert(_love->_buffer.size() <= _n_beats);

    if (!phase_defined) {
      _in->pushBack(in);
      _love->pushBack(new_love);
    } else if (writableAtPosition(timeline_position)) {
      _in->write(getRecordingPosition(timeline_position), in);
   _love->write(getRecordingPosition(timeline_position), new_love);
    }
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
