#pragma once

#include "dsp/misc/signal.hpp"
#include "dsp/Circle/definitions.hpp"
#include "dsp/Circle/Recording.hpp"

namespace kokopellivcv {
namespace emernet {

  /*
   * A LiveNode has a live signal associated which the user inputs audio from.
   */
struct LiveNode : EmernetNode {
  Recording *_in;
  Recording *_love;

  inline LiveNode() {
  }

  inline float getLight() {
  }

  inline ~LiveNode() {
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

  inline Time getRecordingPosition(Time play_head) {
    Time recording_time = play_head;
    recording_time.beat = getMemberBeat(play_head.beat);
    return recording_time;
  }

  inline float readLove(Time play_head) {
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

  inline void write(Time play_head, float in, float love, bool phase_defined) {
    assert(_in->_buffer.size() <= _n_beats);
    assert(_love->_buffer.size() <= _n_beats);
    if (!phase_defined) {
      _in->pushBack(in);
      _love->pushBack(love);
    } else if (writableAtTime(play_head)) {
      _in->write(getRecordingPosition(play_head), in);
    _love->write(getRecordingPosition(play_head), love);
    }
  }

  // inline Time advancePlayHead(Time play_head) {
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

  // inline float step(Time play_head, Inputs inputs, Parameters params) {
  //   if (_recording) {
  //     // if (!phase_defined) {
  //     _in->pushBack(Inputs::readIn(inputs.in, inputs.love));
  //     _love->pushBack(inputs.love);
  //     // } else if (writableAtTime(play_head)) {
  //     //   _in->write(getRecordingPosition(play_head), params.readIn());
  //     //   _love->write(getRecordingPosition(play_head), _inputs.love);
  //     // }
  //     return 0.f;
  //   } else {
  //     return _in->read(getRecordingPosition(play_head));
  //   }
  // }
};

} // namespace emernet
} // namespace kokopellivcv
