#pragma once

#include "definitions.hpp"
#include "Recording.hpp"
#include "Section.hpp"
#include "dsp/Signal.hpp"

#include <numeric> // std::iota

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct Cycle {
  float love = 0.f;

  Section _section_at_start;
  Section* _section;

  Recording *_signal;
  Recording *_love_capture;

  int _write_beat = 0;
  bool _loop = true;
  unsigned int _n_beats = 1;

  inline Cycle(Section* section, kokopellivcv::dsp::SignalType signal_type, int samples_per_beat) {
    _signal = new Recording(signal_type, samples_per_beat);
    _love_capture = new Recording(kokopellivcv::dsp::SignalType::PARAM, samples_per_beat);

    _section = section;
    _section_at_start = *section;
  }

  inline ~Cycle() {
    delete _signal;
    delete _love_capture;
  }

  inline unsigned int getCycleBeat(unsigned int timeline_beat) {
    int beat = timeline_beat - _section->start_beat;
    if (_loop && 0 < beat) {
      return beat % _n_beats;
    }

    return beat;
  }

  inline void updateCyclePeriod(unsigned int n_beats) {
    _signal->updatePeriod(n_beats);
    _love_capture->updatePeriod(n_beats);
    _n_beats = n_beats;
  }

  inline bool readableAtPosition(TimePosition song_position) {
    int cycle_beat = getCycleBeat(song_position.beat);
    return (0 <= cycle_beat && cycle_beat < (int)_n_beats);
  }

  // TODO FIXME allow reverse recording
  inline bool writableAtPosition(TimePosition song_position) {
    return _section->start_beat <= song_position.beat;
  }

  inline TimePosition getRecordingPosition(TimePosition song_position) {
    TimePosition recording_position = song_position;
    recording_position.beat = getCycleBeat(song_position.beat);
    return recording_position;
  }

  inline float listen(TimePosition song_position) {
    if (this->love == 0.f || !readableAtPosition(song_position)) {
      return 0.f;
    }

   return _signal->read(getRecordingPosition(song_position)) * this->love;
  }

  inline float readLove(TimePosition song_position) {
    if (!readableAtPosition(song_position)) {
      return 0.f;
    }

    return _love_capture->read(getRecordingPosition(song_position));
  }

  inline void writeNextBeat() {
    _write_beat++;
    _n_beats++;
  }

  inline void writePrevBeat() {
    _write_beat--;
    _n_beats--;
  }

  inline void write(float phase, float in, float love, bool phase_defined) {
    // TODO why ??
    // assert(_signal->_buffer.size() <= _n_beats);
    // assert(_love_capture->_buffer.size() <= _n_beats);

    if (_write_beat < 0) {
      return;
    }

    TimePosition write_position;
    write_position.beat = _write_beat;
    write_position.phase = phase;

    // FIXME case where recording starts and we go back behind
    if (!phase_defined) {
      _signal->pushBack(in);
      _love_capture->pushBack(love);
    } else {
      _signal->write(write_position, in);
      _love_capture->write(write_position, love);
    }
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
