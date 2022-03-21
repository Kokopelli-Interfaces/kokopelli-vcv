#include "kokopellivcv.hpp"

#include "modules/Hearth.hpp"
#include "modules/Aion.hpp"
#include "modules/Member.hpp"
#include "modules/Ether.hpp"

Plugin *pluginInstance;

void init(rack::Plugin *p) {
	pluginInstance = p;

  p->addModel(modelHearth);
  p->addModel(modelAion);
  p->addModel(modelMember);
  p->addModel(modelEther);
}
