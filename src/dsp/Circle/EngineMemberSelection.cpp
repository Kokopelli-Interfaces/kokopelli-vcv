#include "Engine.hpp"

using namespace kokopellivcv::dsp::circle;
using namespace kokopellivcv::dsp;

// TODO consider member attenuations, so that you can't select fully attenuated member

void Engine::selectRange(unsigned int member_i_1, unsigned int member_i_2) {
  unsigned int i1 = member_i_1;
  unsigned int i2 = member_i_2;
  if (i2 < i1) {
    i2 = i1;
    i1 = member_i_2;
  }

  std::vector<unsigned int> selected_members_idx;
  for (unsigned int i = i1; i <= i2; i++) {
    selected_members_idx.push_back(i);
  }
  _selected_members_idx = selected_members_idx;
}

void Engine::soloSelectMember(unsigned int member_i) {
  _selected_members_idx.erase(_selected_members_idx.begin(), _selected_members_idx.end());
  _selected_members_idx.push_back(member_i);
}

bool Engine::isSolo(unsigned int member_i) {
  return isSelected(member_i) && _selected_members_idx.size() == 1;
}

bool Engine::isSelected(unsigned int member_i) {
  for (auto sel_member_i : _selected_members_idx) {
    if (sel_member_i == member_i) {
      return true;
    }
  }
  return false;
}

void Engine::toggleSelectMember(unsigned int member_i) {
    bool select = true;
    for (unsigned int member_i_i = 0; member_i_i < _selected_members_idx.size(); member_i_i++) {
      if (_selected_members_idx[member_i_i] == _active_member_i) {
        _selected_members_idx.erase(_selected_members_idx.begin() + member_i_i);
        select = false;
      }
    }
    if (select && _timeline.members.size() != 0) {
      _selected_members_idx.push_back(_active_member_i);
    }
}

void Engine::deleteMember(unsigned int member_i) {
  if (member_i < _timeline.members.size()) {
    _timeline.members.erase(_timeline.members.begin()+member_i);
    if (_active_member_i == member_i && _active_member_i != 0) {
      _active_member_i--;
    }
  }

  if (_timeline.members.size() == 0) {
    _phase_oscillator.reset(0.f);
  }
}

void Engine::deleteSelection() {
  if (isRecording()) {
    return;
  }

  for (int member_i = _timeline.members.size()-1; member_i >= 0; member_i--) {
    if (isSelected(member_i)) {
      if (member_i == (int)_active_member_i) {
        _active_member_i = 0 ? 0 : _active_member_i - 1;
      }
      _timeline.members.erase(_timeline.members.begin()+member_i);
    }
  }

  if (_timeline.members.size() == 0) {
    _phase_oscillator.reset(0.f);
  }
}
