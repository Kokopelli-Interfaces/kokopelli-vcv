#include "myrisa.hpp"

#include "Frame.hpp"
#include "Signal.hpp"
#include "Signal4.hpp"
#include "Play.hpp"
#include "FrameX.hpp"

Plugin *pluginInstance;

void init(rack::Plugin *p) {
	pluginInstance = p;

  p->addModel(modelSignal);
  p->addModel(modelFrame);
  p->addModel(modelSignal4);
  p->addModel(modelPlay);
  p->addModel(modelFrameX);
}
