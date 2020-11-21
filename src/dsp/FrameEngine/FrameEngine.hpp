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

  static constexpr int numSections = 16;
  static constexpr float record_threshold = 0.05f;

  Section *active_section = NULL;
  Section *recording_dest_section = NULL;
  Section *sections[numSections]{};

public:
  void startRecording();
  void endRecording();
  void step(float in, float sample_time);
  float read();

  virtual ~FrameEngine() {
    for (auto section : sections) {
      delete section;
    }
  }
};

} // namespace dsp
} // namespace myrisa
