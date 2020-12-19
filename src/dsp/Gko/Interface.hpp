#pragma once

#include "definitions.hpp"

namespace myrisa {
namespace dsp {
namespace gko {

struct Interface {
  std::vector<myrisa::dsp::gko::Connection*> connections;

  float sample_time = 1.0f;

  bool use_ext_phase = false;
  float ext_phase = 0.f;

  unsigned int active_layer_i;
  std::vector<unsigned int> selected_layers_idx;
  bool select_new_layers = true;
  bool new_layer_active = true;

  RecordInterface record_interface;
  TimeFrame read_time_frame;

  Options options;
};

} // namespace gko
} // namespace dsp
} // namepsace myrisa
