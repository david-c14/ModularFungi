#include "ModularFungi.hpp"

template<int x>
struct BlankWidget : ModuleWidget {
	std::string FileName() {
		char workingSpace[100];
		snprintf(workingSpace, 100, "res/Blank_%dHP.png", x);
		return assetPlugin(plugin, workingSpace);
	}

	BlankWidget(Module *module) : ModuleWidget(module) {
		box.size = Vec(RACK_GRID_WIDTH * x, RACK_GRID_HEIGHT);
		BitMap * bmp = Widget::create<BitMap>(Vec(0,0));
		bmp->box.size.x = box.size.x;
		bmp->box.size.y = box.size.y;
		bmp->path = FileName();
		addChild(bmp);
	}
};

#define MODEL(x) Model *modelBlank_##x##HP = Model::create<Module, BlankWidget<x>>("Modular Fungi", "Blank " #x "HP", #x "HP Blanking Plate", BLANK_TAG);
MODEL(1)
MODEL(3)
MODEL(4)
MODEL(6)
MODEL(10)
MODEL(12)
MODEL(16)
MODEL(20)
MODEL(26)
MODEL(32)
