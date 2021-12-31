#include "kokopellivcv.hpp"
#include "modules/Circle.hpp"
#include "modules/Aion.hpp"

Plugin *pluginInstance;

void init(rack::Plugin *p) {
	pluginInstance = p;

  p->addModel(modelCircle);
  p->addModel(modelAion);
}
