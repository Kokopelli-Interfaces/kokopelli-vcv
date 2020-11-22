#include "FrameEngine.hpp"

using namespace myrisa::dsp;

FrameEngine::FrameEngine() {
  for (int i = 0; i < numSections; i++) {
    _sections.push_back(new Section(this));
  }
}

void FrameEngine::handleModeChange() {
  if (_active_section) {
    printf("MODE CHANGE:: %d -> %d\n", _prev_mode, _mode);
    _active_section->setRecordMode(_mode);
    _prev_mode = _mode;
  }
}

void FrameEngine::updateSectionPosition(float section_position) {
  _section_position = section_position;
  int active_section_i = round(section_position);
  if (active_section_i == numSections) {
    active_section_i--;
  }
  _active_section = _sections[active_section_i];
}

void FrameEngine::step() {
  if (_prev_mode != _mode) {
    handleModeChange();
  }

  for (auto section : _sections) {
    section->step();
  }
}

float FrameEngine::read() {
  int section_1 = floor(_section_position);
  int section_2 = ceil(_section_position);
  float weight = _section_position - floor(_section_position);

  float out = 0.0f;
  out += _sections[section_1]->read() * (1 - weight);
  if (section_1 != section_2 && section_2 < numSections) {
    out += _sections[section_2]->read() * weight;
  }

  return out;
}
