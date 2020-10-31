#pragma once
#include <rack.hpp>

using namespace rack;
using namespace myrisa;

// Declare the Plugin, defined in plugin.cpp
extern Plugin *pluginInstance;

// Declare each Model, defined in each module source file
// extern Model *modelMyModule;
extern Model *modelMyrisaMidiCC;
extern Model *modelSignal;
extern Model *modelMyrisaSignal4;
extern Model *modelPlay;
extern Model *modelFrame;
extern Model *modelFrameX;
