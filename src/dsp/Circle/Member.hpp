#pragma once

#include "definitions.hpp"
#include "Recording.hpp"
#include "dsp/Signal.hpp"

namespace kokpelliinterfaces {
namespace dsp {
namespace circle {

struct Member {
  unsigned int _n_beats = 0;
  unsigned int _beat = 0;

  bool _loop = false;

  // TODO
  // std::vector<Recording*> recordings;
  // std::vector<kokpelliinterfaces::dsp::SignalType> types;

  Recording *_in;
  Recording *_love;

  // FIXME change me to be an array of bools for O(1) lookup
  std::vector<unsigned int> members_being_observed_idx;

  inline Member(unsigned int n_beats, std::vector<unsigned int> members_being_observed_idx, kokpelliinterfaces::dsp::SignalType signal_type, int samples_per_beat) {
    this->_n_beats = n_beats;
    this->members_being_observed_idx = members_being_observed_idx;

    _in = new Recording(signal_type, samples_per_beat);
    _love = new Recording(kokpelliinterfaces::dsp::SignalType::PARAM, samples_per_beat);
  }

  inline ~Member() {
    delete _in;
    delete _love;
  }

  // inline unsigned int getMemberBeat(unsigned int timeline_beat) {
  //   int beat = timeline_beat - _start_beat;
  //   if (_loop && 0 < beat) {
  //     return beat % _n_beats;
  //   }

  //   return beat;
  // }

  inline bool readableAtPosition(TimePosition timeline_position) {
    int member_beat = getMemberBeat(timeline_position.beat);
    return (0 <= member_beat && member_beat < (int)_n_beats);
  }

  // // TODO FIXME allow reverse recording
  // inline bool writableAtPosition(TimePosition timeline_position) {
  //   return _start_beat <= timeline_position.beat && timeline_position.beat <= _start_beat + _n_beats;
  // }

  // inline TimePosition getRecordingPosition(TimePosition timeline_position) {
  //   TimePosition recording_position = timeline_position;
  //   recording_position.beat = getMemberBeat(timeline_position.beat);
  //   return recording_position;
  // }

  // inline float readRecordingLove(TimePosition timeline_position) {
  //   if (!readableAtPosition(timeline_position)) {
  //     return 0.f;
  //   }

  //   return _love->read(getRecordingPosition(timeline_position));
  // }

  // inline void write(TimePosition timeline_position, float in, float love, bool phase_defined) {
  //   assert(_in->_buffer.size() <= _n_beats);
  //   assert(_love->_buffer.size() <= _n_beats);

  //   if (!phase_defined) {
  //     _in->pushBack(in);
  //     _love->pushBack(love);
  //   } else if (writableAtPosition(timeline_position)) {
  //     _in->write(getRecordingPosition(timeline_position), in);
  //  _love->write(getRecordingPosition(timeline_position), love);
  //   }
  // }

  inline prevBeat() {
  }

  inline nextBeat() {
  }

  inline float sing(float phase) {
    if (!readableAtPosition(time)) {
      return 0.f;
    }
    return _in->read(getRecordingPosition(time));
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokpelliinterfaces
