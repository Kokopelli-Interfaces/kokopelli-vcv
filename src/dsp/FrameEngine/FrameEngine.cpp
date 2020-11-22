#include "FrameEngine.hpp"

using namespace myrisa::dsp;

FrameEngine::FrameEngine() {
  for (int i = 0; i < numSections; i++) {
    sections.push_back(new Section(this));
  }
}

// TODO customizable respoonse?
// TODO attenuation clk divider, get smooth value
inline float getAttenuationPower(float delta, float recording_threshold) {
  float read_position = 0.50f;
  float linear_attenuation_power;
  if (delta < read_position - recording_threshold) {
    linear_attenuation_power = read_position - recording_threshold - delta;
  } else if (delta > read_position - recording_threshold) {
    linear_attenuation_power = delta - read_position + recording_threshold;
  } else {
    linear_attenuation_power = 0.0f;
  }

  float range = read_position - recording_threshold;
  float linear_attenuation_power_scaled = linear_attenuation_power / range;

  // I found that taking to the power 3 gives the most intuitive attenuation
  // power curve
  float attenuation_power =
      rack::clamp(pow(linear_attenuation_power_scaled, 3), 0.0f, 1.0f);
  return attenuation_power;
}

void FrameEngine::startRecording() {
  _recording = true;
  recording_dest_section = _active_section;

  if (recording_dest_section->isEmpty() && !_use_ext_phase) {
    recording_dest_section->setRecordMode(RecordMode::DEFINE_DIVISION_LENGTH);
  } else if (_delta > 0.50f + record_threshold) {
    recording_dest_section->setRecordMode(RecordMode::EXTEND);
  } else {
    recording_dest_section->setRecordMode(RecordMode::DUB);
  }
}

void FrameEngine::endRecording() {
  _recording = false;
  recording_dest_section->setRecordMode(RecordMode::READ);
}

void FrameEngine::step() {
  int active_section_i = round(_section_position);
  _active_section = sections[active_section_i];

  bool delta_engaged = record_threshold <= std::fabs(_delta - .50);
  if (!_recording && delta_engaged) {
    startRecording();
  } else if (_recording && !delta_engaged) {
    endRecording();
  }

  for (auto section : sections) {
    section->step();
  }
}

float FrameEngine::read() {
  int section_1 = floor(_section_position);
  int section_2 = ceil(_section_position);
  float weight = _section_position - floor(_section_position);

  float out = 0.0f;
  out += sections[section_1]->read() * (1 - weight);
  if (section_1 != section_2 && section_2 < numSections) {
    out += sections[section_2]->read() * (weight);
  }

  return out;
}
