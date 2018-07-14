#include "OC_Blanks.hpp"

struct Blank1 : ModuleWidget {
	Blank1(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Blank1.svg")));
	}
};

struct Blank2 : ModuleWidget {
	Blank2(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Blank2.svg")));
	}
};

struct Blank3 : ModuleWidget {
	Blank3(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Blank3.svg")));
	}
};

Model *modelBlank1 = Model::create<Module, Blank1>("OC_Blanks", "Blank1", "Blanking Plate Number 1", BLANK_TAG);
Model *modelBlank2 = Model::create<Module, Blank2>("OC_Blanks", "Blank2", "Blanking Plate Number 2", BLANK_TAG);
Model *modelBlank3 = Model::create<Module, Blank3>("OC_Blanks", "Blank3", "Blanking Plate Number 3", BLANK_TAG);
