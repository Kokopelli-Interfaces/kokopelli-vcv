#include "Engine.hpp"

using namespace kokopelli::dsp::circle;
using namespace kokopelli::dsp;

void Engine::reflect() {
  if (_loving) {
    _focused_member->_loop = true;
    // TODO end recording
    Member* new_member = new Member(nullptr, _signal_type);
    _focused_member = new_member;
    _cicle->_members.push_back(new_member);
  } else {
    // TODO alternate trim mode?
    // pedal is like solo / mute
  }
}

void Engine::prevMember() {
  // TODO
  if (_loving) {
    // TODO create circle with prev member
  } else {
    // TODO left
  }
}

void Engine::nextMember() {
  if (_loving) {
    // TODO end recording
    Member* new_member = new Member(nullptr, _signal_type);
    _focused_member = new_member;
    _cicle->_members.push_back(new_member);
  } else {
    // TODO right
  }
}

void Engine::step() {
  if (!_loving && interface->isLoving()) {
    _loving = true;

    Member* new_member = new Member(nullptr, _signal_type);

    // TODO more than one circle (sound source class?)
    if (_cicle->_members.size() == 0) {
      _cicle->_members.push_back(new_member);
    } else {
      // TODO create new circle
      // _focused_member + new_member -> new circle
      _cicle->_members.push_back(new_member);
    }

    _focused_member = new_member;
  } else if (_loving && !interface->isLoving()) {
    _loving = false;
  }

  if (_loving) {
    _focused_member->listen(interface->in, interface->love);
  }

  for (auto member : _cicle) {
    return member->advance();
  }
}
