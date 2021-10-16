#pragma once

#include "CircleVoice.hpp"
#include "definitions.hpp"
#include "Recording.hpp"
#include "dsp/Signal.hpp"

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct CircleMember {
  TimePosition _start;

  unsigned int _n_beats = 0;
  bool _loop = false;

  // TODO multiple I/O modules on left
  // std::vector<Recording*> recordings;
  // std::vector<kokopellivcv::dsp::SignalType> types;

  Recording *_in;
  Recording *_love;

  inline CircleMember(TimePosition start, unsigned int n_beats) {
    this->_start = start;
    this->_n_beats = n_beats;

    _in = new Recording(signal_type, samples_per_beat);
    _love = new Recording(kokopellivcv::dsp::SignalType::PARAM, samples_per_beat);
  }

  inline ~CircleMember() {
    delete _in;
    delete _love;
  }

  inline bool isLooping() {
    return _loop;
  }

  inline void fitToLength(unsigned int length) {
    _n_beats = length;
  }

  inline void setLoop(bool loop) {
    _loop = loop;
  }

  inline unsigned int getMemberBeat(unsigned int timeline_beat) {
    int beat = timeline_beat - _start.beat;
    if (_loop && 0 < beat) {
      return beat % _n_beats;
    }

    return beat;
  }

  inline bool readableAtTime(TimePosition time) {
    int member_beat = getMemberBeat(time.beat);
    return (0 <= member_beat && member_beat < (int)_n_beats);
  }

  // TODO FIXME allow reverse recording
  inline bool writableAtTime(TimePosition time) {
    return _start.beat <= time.beat && time.beat <= _start.beat + _n_beats;
  }

  inline TimePosition getRecordingTime(TimePosition time) {
    TimePosition recording_time = time;
    recording_time.beat = getMemberBeat(time.beat);
    return recording_time;
  }

  inline float readSignal(TimePosition time) {
    if (!readableAtTime(time)) {
      return 0.f;
    }

    return _in->read(getRecordingTime(time));
  }

  inline float readLove(TimePosition time) {
    if (!readableAtTime(time)) {
      return 0.f;
    }

    return _love->read(getRecordingTime(time));
  }

  inline void write(TimePosition time, float in, float new_love, bool phase_defined) {
    assert(_in->_buffer.size() <= _n_beats);
    assert(_love->_buffer.size() <= _n_beats);

    if (!phase_defined) {
      _in->pushBack(in);
      _love->pushBack(new_love);
    } else if (writableAtTime(time)) {
      _in->write(getRecordingTime(time), in);
   _love->write(getRecordingTime(time), new_love);
    }
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
