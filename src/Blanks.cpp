#include "ModularFungi.hpp"

struct BitMap : TransparentWidget {
	std::string path;
	int loaded = false;
	int bitmap = 0;
	NVGcontext *storedVG;
	void DrawImage(NVGcontext *vg) {
		storedVG = vg;	
		if (!loaded) {
			loaded = true;
			bitmap = nvgCreateImage(vg, path.c_str(), NVG_IMAGE_GENERATE_MIPMAPS);
			if (!bitmap)
				warn("ModularFungi: Unable to load %s", path.c_str());
		}
		if (!bitmap)
			return;	
		NVGpaint paint = nvgImagePattern(vg, 0, 0, box.size.x, box.size.y, 0.0f, bitmap, 1.0f);
		nvgFillPaint(vg, paint);
		nvgBeginPath(vg);
		nvgRect(vg, 0, 0, box.size.x, box.size.y);
		nvgFill(vg);
		
	}
	void draw(NVGcontext *vg) override {
		DrawImage(vg);
		TransparentWidget::draw(vg);
	}
	~BitMap() {
		if (!bitmap)
			nvgDeleteImage(storedVG, bitmap);
	}
};

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
