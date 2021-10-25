#pragma once

#include "definitions.hpp"
#include "Recording.hpp"
#include "dsp/Signal.hpp"

#include <numeric> // std::iota

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct Member {
  unsigned int _start_beat = 0;
  unsigned int _n_beats = 0;
  bool _loop = false;

  std::pair<unsigned int, unsigned int> _circle_before = std::make_pair(0, 1);

  // TODO
  // std::vector<Recording*> recordings;
  // std::vector<kokopellivcv::dsp::SignalType> types;

  Recording *_in;
  Recording *_recording_strength;

  // FIXME change me to be an array of bools for O(1) lookup
  std::vector<unsigned int> target_members_idx;

  inline Member(unsigned int start_beat, unsigned int n_beats, std::vector<unsigned int> target_members_idx, kokopellivcv::dsp::SignalType signal_type, int samples_per_beat) {
    this->_start_beat = start_beat;
    this->_n_beats = n_beats;
    this->target_members_idx = target_members_idx;

    _in = new Recording(signal_type, samples_per_beat);
    _recording_strength = new Recording(kokopellivcv::dsp::SignalType::PARAM, samples_per_beat);
  }

  inline ~Member() {
    delete _in;
    delete _recording_strength;
  }

  inline unsigned int getMemberBeat(unsigned int timeline_beat) {
    int beat = timeline_beat - _start_beat;
    if (_loop && 0 < beat) {
      return beat % _n_beats;
    }

    return beat;
  }

  inline bool readableAtPosition(TimePosition timeline_position) {
    int member_beat = getMemberBeat(timeline_position.beat);
    return (0 <= member_beat && member_beat < (int)_n_beats);
  }

  // TODO FIXME allow reverse recording
  inline bool writableAtPosition(TimePosition timeline_position) {
    return _start_beat <= timeline_position.beat && timeline_position.beat <= _start_beat + _n_beats;
  }

  inline TimePosition getRecordingPosition(TimePosition timeline_position) {
    TimePosition recording_position = timeline_position;
    recording_position.beat = getMemberBeat(timeline_position.beat);
    return recording_position;
  }

  inline float readSignal(TimePosition timeline_position) {
    if (!readableAtPosition(timeline_position)) {
      return 0.f;
    }

    return _in->read(getRecordingPosition(timeline_position));
  }

  inline float readRecordingStrength(TimePosition timeline_position) {
    if (!readableAtPosition(timeline_position)) {
      return 0.f;
    }

    return _recording_strength->read(getRecordingPosition(timeline_position));
  }

  inline void write(TimePosition timeline_position, float in, float strength, bool phase_defined) {
    assert(_in->_buffer.size() <= _n_beats);
    assert(_recording_strength->_buffer.size() <= _n_beats);

    if (!phase_defined) {
      _in->pushBack(in);
      _recording_strength->pushBack(strength);
    } else if (writableAtPosition(timeline_position)) {
      _in->write(getRecordingPosition(timeline_position), in);
   _recording_strength->write(getRecordingPosition(timeline_position), strength);
    }
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
