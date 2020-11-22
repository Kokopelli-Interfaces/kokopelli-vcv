#pragma once

#include "Section.hpp"
#include "definitions.hpp"
#include "dsp/PhaseAnalyzer.hpp"
#include "Layer.hpp"
#include "rack.hpp"
#include "util/assert.hpp"
#include <assert.h>
#include <math.h>
#include <vector>

    namespace myrisa {
namespace dsp {
namespace frame {

class Engine {
public:
  bool _use_ext_phase = false;
  float _ext_phase = 0.0f;
  float _in = 0.0f;
  float _sample_time = 1.0f;
  float _section_position = 0.0f;
  float _attenuation = 0.0f;
  RecordMode _mode = RecordMode::READ;
  Section *_active_section = nullptr;

private:
  const int numSections = 16;
  Section *recording_dest_section = nullptr;
  std::vector<Section*> _sections;
  RecordMode _active_mode = RecordMode::READ;

  PhaseAnalyzer _phase_analyzer;

public:
  Engine();
  void updateSectionPosition(float section_position);
  void step();
  float read();

private:
  void handleModeChange();
  void stepSection(Section *section);
  bool addLayer(Section *section, RecordMode mode);
};

} // namespace frame
} // namespace dsp
} // namespace myrisa
