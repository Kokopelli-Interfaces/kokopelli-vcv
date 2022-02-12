#include "kokopellivcv.hpp"
#include "modules/Hearth.hpp"
#include "modules/Aion.hpp"

Plugin *pluginInstance;

void init(rack::Plugin *p) {
	pluginInstance = p;

  p->addModel(modelHearth);
  p->addModel(modelAion);
}
