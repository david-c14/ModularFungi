#include "ModularFungi.hpp"

struct ColorWidget : ModuleWidget {
	BitMap *bmp;

	ColorWidget(Module *module) : ModuleWidget() {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * 12, RACK_GRID_HEIGHT);
		loadBitmap();
	}
	void loadBitmap() {
		bmp = createWidget<BitMap>(Vec(0,0));
		bmp->box.size.x = box.size.x;
		bmp->box.size.y = box.size.y;
		bmp->path = asset::plugin(pluginInstance, "res/Colors.png");
		addChild(bmp);
	}
};

Model *modelColor_12HP = createModel<Module, ColorWidget>("Color12HP");
