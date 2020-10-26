
#include "myrisa.hpp"

#include "Frame.hpp"
#include "Signal.hpp"
#include "4Signal.hpp"
#include "Play.hpp"
#include "FrameX.hpp"

Plugin *pluginInstance;

void init(rack::Plugin *p) {
	pluginInstance = p;

  p->addModel(modelFrame);
  p->addModel(modelSignal);
  p->addModel(model4Signal);
  p->addModel(modelPlay);
  p->addModel(modelFrameX);
}
