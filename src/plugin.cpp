#include "plugin.hpp"

Plugin *pluginInstance;

void init(Plugin *p) {
  pluginInstance = p;

  p->addModel(modelMyrisaMidiCC);
  p->addModel(modelMyrisaSignal);
  p->addModel(modelMyrisa4Signal);
  p->addModel(modelMyrisaPlay);
  p->addModel(modelMyrisaFrame);
  p->addModel(modelMyrisaFrameX);
}
