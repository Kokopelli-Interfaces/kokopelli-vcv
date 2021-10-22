#include "kokopellivcv.hpp"

#include "modules/Circle.hpp"
#include "modules/Signal.hpp"

Plugin *pluginInstance;

void init(rack::Plugin *p) {
	pluginInstance = p;

  p->addModel(modelSignal);
  p->addModel(modelCircle);
}
