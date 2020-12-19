#pragma once

#include "dsp/AntipopFilter.hpp"
#include "definitions.hpp"
#include "Layer.hpp"
#include "Channel.hpp"

namespace myrisa {
namespace dsp {
namespace gko {

/**
   Records a connection into a Layer.
  */
class Recorder {
public:
  Layer *_recording_layer = nullptr;
  RecordInterface record_interface;
  AntipopFilter _write_antipop_filter;

public:
  inline bool isRecording() {
    return _recording_layer != nullptr;
  }

  inline Layer* endRecording() {
    assert(isRecording());
    assert(_recording_layer->_n_beats != 0.f);

    printf("- rec end\n");
    printf("-- start_beat %d n_beats %d  loop %d samples_per_beat %d\n", _recording_layer->_start_beat, _recording_layer->_n_beats,  _recording_layer->_loop, _recording_layer->_in->_samples_per_beat);

    if (!_phase_oscillator.isSet()) {
      if (_use_ext_phase && _phase_analyzer.getDivisionPeriod() != 0) {
        _phase_oscillator.setFrequency(1 / _phase_analyzer.getDivisionPeriod());
      } else {
        float recording_time = _recording_layer->_in->_samples_per_beat * _sample_time;
        _phase_oscillator.setFrequency(1 / recording_time);
      }
      printf("-- phase oscillator set with frequency: %f, sample time is: %f\n", _phase_oscillator.getFrequency(), _sample_time);
    }

    _timeline.layers.push_back(_recording_layer);
        _timeline._last_calculated_attenuation.resize(_timeline.layers.size());
    _timeline._current_attenuation.resize(_timeline.layers.size());

    unsigned int layer_i = _timeline.layers.size() - 1;

    if (_select_new_layers) {
      _selected_layers_idx.push_back(layer_i);
    }

    if (_new_layer_active) {
      _active_layer_i = layer_i;
    }

    _recording_layer = nullptr;
  }

  inline newRecording(TimePosition timeline_position, RecordInterface record_interface, int samples_per_beat) {
    assert(record_interface.active());
    assert(_recording_layer == nullptr);

    _write_antipop_filter.trigger();

    unsigned int n_beats = 1;
    if (record_interface.mode == RecordInterface::Mode::DUB && 0 < _timeline.layers.size()) {
      n_beats = _timeline.layers[_active_layer_i]->_n_beats;
    }

    unsigned int start_beat = _timeline_position.beat;

    int samples_per_beat = 0;
    if (phaseDefined()) {
      if (_use_ext_phase) {
        samples_per_beat = _phase_analyzer.getSamplesPerBeat(_sample_time);
      } else if (_phase_oscillator.isSet()) {
        float beat_period = 1 / _phase_oscillator.getFrequency();
        samples_per_beat = floor(beat_period / _sample_time);
      }
    }

    _recording_layer = new Layer(start_beat, n_beats, _selected_layers_idx, _signal_type, samples_per_beat);

    if (record_interface.time_frame != TimeFrame::TIMELINE) {
      _recording_layer->_loop = true;
    }

    printf("Recording Activate:\n");
    printf("-- start_beat %d n_beats %d loop %d samples per beat %d active layer %d\n", _recording_layer->_start_beat, _recording_layer->_n_beats, _recording_layer->_loop, _recording_layer->_in->_samples_per_beat, _active_layer_i);

    _recording_layer;
  }
};


} // namespace gko
} // namespace dsp
} // namepsace myrisa
