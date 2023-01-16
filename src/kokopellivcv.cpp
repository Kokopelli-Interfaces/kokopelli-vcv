#include "kokopellivcv.hpp"

#include "modules/Circle.hpp"
// #include "modules/Aion.hpp"
// #include "modules/Member.hpp"
// #include "modules/Ether.hpp"

Plugin *pluginInstance;

void init(rack::Plugin *p) {
	pluginInstance = p;

  p->addModel(modelCircle);
  // p->addModel(modelAion);
  // p->addModel(modelMember);
  // p->addModel(modelEther);
}
