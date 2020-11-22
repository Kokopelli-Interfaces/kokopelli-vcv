#include "FrameEngine.hpp"

using namespace myrisa::dsp;

FrameEngine::FrameEngine() {
  for (int i = 0; i < numSections; i++) {
    sections.push_back(new Section());
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
  recording = true;
  recording_dest_section = active_section;

  if (recording_dest_section->isEmpty() && !use_ext_phase) {
    recording_dest_section->setRecordMode(RecordMode::DEFINE_DIVISION_LENGTH);
  } else if (delta > 0.50f + record_threshold) {
    recording_dest_section->setRecordMode(RecordMode::EXTEND);
  } else {
    recording_dest_section->setRecordMode(RecordMode::DUB);
  }
}

void FrameEngine::endRecording() {
  recording = false;
  recording_dest_section->setRecordMode(RecordMode::READ);
}

void FrameEngine::step(float in, float sample_time) {
  int active_section_i = round(section_position);
  active_section = sections[active_section_i];

  bool delta_engaged = record_threshold <= std::fabs(delta - .50);
  if (!recording && delta_engaged) {
    startRecording();
  } else if (recording && !delta_engaged) {
    endRecording();
  }

  for (auto section : sections) {
    section->step(in, attenuation, sample_time, use_ext_phase, ext_phase);
  }
}

float FrameEngine::read() {
  int section_1 = floor(section_position);
  int section_2 = ceil(section_position);
  float weight = section_position - floor(section_position);

  float out = 0.0f;
  out += sections[section_1]->read() * (1 - weight);
  if (section_1 != section_2 && section_2 < numSections) {
    out += sections[section_2]->read() * (weight);
  }

  return out;
}
