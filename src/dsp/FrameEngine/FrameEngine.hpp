#pragma once

#include "Section.hpp"

using namespace myrisa::dsp;
using namespace std;

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

  constexpr int numSections = 16;
  const float record_threshold = 0.05f;

  Section *active_section = nullptr;
  Section *recording_dest_section = nullptr;
  array<Section*, numSections> sections;

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
