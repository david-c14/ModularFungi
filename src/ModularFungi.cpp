#include "ModularFungi.hpp"


Plugin *pluginInstance;


void init(rack::Plugin *p) {
	pluginInstance = p;

	// Add all Models defined throughout the pluginInstance
	p->addModel(modelBlank_1HP);
	p->addModel(modelBlank_3HP);
	p->addModel(modelBlank_4HP);
	p->addModel(modelBlank_6HP);
	p->addModel(modelBlank_10HP);
	p->addModel(modelBlank_12HP);
	p->addModel(modelBlank_16HP);
	p->addModel(modelBlank_20HP);
	p->addModel(modelBlank_26HP);
	p->addModel(modelBlank_32HP);
	p->addModel(modelColor_12HP);
	p->addModel(modelLightsOff);

	// Any other pluginInstance initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
