#pragma once

#include "definitions.hpp"
#include "Recording.hpp"
#include "dsp/Signal.hpp"

namespace tribalinterfaces {
namespace dsp {
namespace circle {

struct Member {
  unsigned int _start_beat = 0;
  unsigned int _n_beats = 0;
  bool _loop = false;

  // TODO
  // std::vector<Recording*> recordings;
  // std::vector<tribalinterfaces::dsp::SignalType> types;

  Recording *_in;
  Recording *_recording_love;

  // FIXME change me to be an array of bools for O(1) lookup
  std::vector<unsigned int> target_members_idx;

  inline Member(unsigned int start_beat, unsigned int n_beats, std::vector<unsigned int> target_members_idx, tribalinterfaces::dsp::SignalType signal_type, int samples_per_beat) {
    this->_start_beat = start_beat;
    this->_n_beats = n_beats;
    this->target_members_idx = target_members_idx;

    _in = new Recording(signal_type, samples_per_beat);
    _recording_love = new Recording(tribalinterfaces::dsp::SignalType::PARAM, samples_per_beat);
  }

  inline ~Member() {
    delete _in;
    delete _recording_love;
  }

  inline unsigned int getMemberBeat(unsigned int circle_beat) {
    int beat = circle_beat - _start_beat;
    if (_loop && 0 < beat) {
      return beat % _n_beats;
    }

    return beat;
  }

  inline bool readableAtPosition(TimePosition circle_position) {
    int member_beat = getMemberBeat(circle_position.beat);
    return (0 <= member_beat && member_beat < (int)_n_beats);
  }

  // TODO FIXME allow reverse recording
  inline bool writableAtPosition(TimePosition circle_position) {
    return _start_beat <= circle_position.beat && circle_position.beat <= _start_beat + _n_beats;
  }

  inline TimePosition getRecordingPosition(TimePosition circle_position) {
    TimePosition recording_position = circle_position;
    recording_position.beat = getMemberBeat(circle_position.beat);
    return recording_position;
  }

  inline float readSignal(TimePosition circle_position) {
    if (!readableAtPosition(circle_position)) {
      return 0.f;
    }

    return _in->read(getRecordingPosition(circle_position));
  }

  inline float readRecordingLove(TimePosition circle_position) {
    if (!readableAtPosition(circle_position)) {
      return 0.f;
    }

    return _recording_love->read(getRecordingPosition(circle_position));
  }

  inline void write(TimePosition circle_position, float in, float love, bool phase_defined) {
    assert(_in->_buffer.size() <= _n_beats);
    assert(_recording_love->_buffer.size() <= _n_beats);

    if (!phase_defined) {
      _in->pushBack(in);
      _recording_love->pushBack(love);
    } else if (writableAtPosition(circle_position)) {
      _in->write(getRecordingPosition(circle_position), in);
   _recording_love->write(getRecordingPosition(circle_position), love);
    }
  }
};

} // namespace circle
} // namespace dsp
} // namespace tribalinterfaces
