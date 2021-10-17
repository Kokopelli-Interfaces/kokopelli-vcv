#pragma once

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

  bool _recording = false;

  // TODO multiple I/O modules on left
  // std::vector<Recording*> recordings;
  // std::vector<kokopellivcv::dsp::SignalType> types;

  Recording *_in;
  Recording *_love;

  inline CircleMember() {
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
    return beat % _n_beats;
  }

  inline TimePosition getRecordingPosition(TimePosition play_head) {
    TimePosition recording_time = play_head;
    recording_time.beat = getMemberBeat(play_head.beat);
    return recording_time;
  }

  inline float readLove(TimePosition play_head) {
    if (!readableAtTime(play_head)) {
      return 0.f;
    }
    return _love->read(getRecordingPosition(play_head));
  }

  inline void clearMemory() {
    if (_in) {
      delete _in;
    }
    if (_love) {
      delete _love;
    }
  }

  inline void startRecording(Parameters params) {

    clearMemory();

    // FIXME
    _in = new Recording(params.signal_type, params.samples_per_beat);
    _love = new Recording(kokopellivcv::dsp::SignalType::PARAM, params.samples_per_beat);
    _recording = true;
  }

  inline void stopRecording() {
    assert(_recording);
    _recording = false;
  }

  // inline void write(TimePosition play_head, float in, float love, bool phase_defined) {
  //   assert(_in->_buffer.size() <= _n_beats);
  //   assert(_love->_buffer.size() <= _n_beats);
  //   if (!phase_defined) {
  //     _in->pushBack(in);
  //     _love->pushBack(love);
  //   } else if (writableAtTime(play_head)) {
  //     _in->write(getRecordingPosition(play_head), in);
  //   _love->write(getRecordingPosition(play_head), love);
  //   }
  // }

  // inline TimePosition advancePlayHead(TimePosition play_head) {
  //   _play_head.phase = play_head.phase;
  //   if (_last_play_head.beat != play_head.beat) {
  //     unsigned int n_group_beats = this->getPeriod();
  //     if (n_group_beats != 0) {
  //       _play_head.beat = _last_play_head.beat % n_group_beats;
  //     } else {
  //       _play_head.beat = 0;
  //     }
  //   }
  //   _last_play_head = play_head;
  // }

  inline float step(TimePosition play_head, Inputs inputs, Parameters params) {
    if (_recording) {
      // if (!phase_defined) {
      _in->pushBack(Inputs::readIn(inputs.in, inputs.love));
      _love->pushBack(inputs.love);
      // } else if (writableAtTime(play_head)) {
      //   _in->write(getRecordingPosition(play_head), params.readIn());
      //   _love->write(getRecordingPosition(play_head), _inputs.love);
      // }
      return 0.f;
    } else {
      return _in->read(getRecordingPosition(play_head));
    }
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
