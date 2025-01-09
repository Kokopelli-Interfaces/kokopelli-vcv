#include "kokopellivcv.hpp"

#include "modules/Circle.hpp"

Plugin *pluginInstance;

void init(rack::Plugin *p) {
	pluginInstance = p;

  p->addModel(modelCircle);
}
