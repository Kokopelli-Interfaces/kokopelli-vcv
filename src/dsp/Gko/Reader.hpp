
#pragma once

#include "dsp/AntipopFilter.hpp"
#include "definitions.hpp"
#include "Layer.hpp"

namespace myrisa {
namespace dsp {
namespace gko {

class Reader {
  AntipopFilter _read_antipop_filter;

public:
  inline void triggerAntipop() {
    _read_antipop_filter.trigger();
  }

  inline void read(Timeline timeline, TimePosition timeline_position) {
    float timeline_out = timeline.read(timeline_position, _recording_layer, _record_interface);

    timeline_out = _read_antipop_filter.process(timeline_out);

    return timeline_out;
  }

  inline void handlePhaseEvent(PhaseAnalyzer::PhaseEvent event, bool use_antipop) {
    if (use_antipop && evet == PhaseAnalyzer::PhaseEvent::DISCONTINUITY) {
      _read_antipop_filter.trigger();
    }
  }
};

} // namespace gko
} // namespace dsp
} // namepsace myrisa
