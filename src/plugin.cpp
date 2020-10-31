#include "plugin.hpp"

Plugin *pluginInstance;

void init(Plugin *p) {
  pluginInstance = p;

  p->addModel(modelMyrisaMidiCC);
  p->addModel(modelSignal);
  p->addModel(modelMyrisaSignal4);
  p->addModel(modelPlay);
  p->addModel(modelFrame);
  p->addModel(modelFrameX);
}
