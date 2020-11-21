#pragma once

#include "Section.hpp"

using namespace myrisa::dsp;

namespace myrisa {
namespace dsp {

class FrameEngine {
public:
  float section_position = 0.0f;
  float delta = 0.0f;
  bool recording = false;
  bool use_ext_phase = false;
  float ext_phase = 0.0f;
  float attenuation = 0.0f;

  const int numSections = 16;
  const float record_threshold = 0.05f;

  Section *active_section = nullptr;
  Section *recording_dest_section = nullptr;

  std::vector<Section> sections;

public:
  FrameEngine() {
    for (int i=0; i<numSections; i++) {
      sections.push_back(Section());
    }
  }

  void startRecording();
  void endRecording();
  void step(float in, float sample_time);
  float read();
};

} // namespace dsp
} // namespace myrisa
