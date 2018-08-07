#include "ModularFungi.hpp"

struct BitMap : TransparentWidget {
	std::string path;
	int loaded = false;
	int bitmap;
	void DrawImage(NVGcontext *vg) {
		if (!loaded) {
			loaded = true;
			bitmap = nvgCreateImage(vg, path.c_str(), NVG_IMAGE_GENERATE_MIPMAPS);
			if (!bitmap)
				return;
		
			int w, h;
			nvgImageSize(vg, bitmap, &w, &h);
			box.size.x = w;
			box.size.y = h;	
			box.pos.x = (parent->box.size.x - w) / 2;
			box.pos.y = (parent->box.size.y - h) / 2;
			return;
		}
		if (!loaded)
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
};

template<int x>
struct BlankWidget : ModuleWidget {
	std::string FileName() {
		char workingSpace[100];
		snprintf(workingSpace, 100, "res/Blank_%dHP.png", x);
		debug("%s", workingSpace);
		return assetPlugin(plugin, workingSpace);
	}

	BlankWidget(Module *module) : ModuleWidget(module) {
		box.size = Vec(RACK_GRID_WIDTH * x, RACK_GRID_HEIGHT);
		BitMap * bmp = Widget::create<BitMap>(Vec(0,0));
		bmp->path = FileName();
		addChild(bmp);
	}
};

struct Blank_New_1 : ModuleWidget {
	Blank_New_1(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Blank_New_1.svg")));
		BitMap * bmp = Widget::create<BitMap>(Vec(0,0));
		bmp->path = assetPlugin(plugin, "res/Blank_New_1.png");
		addChild(bmp);
	}
};

struct Blank_New_2 : ModuleWidget {
	Blank_New_2(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Blank_New_2.svg")));
		BitMap * bmp = Widget::create<BitMap>(Vec(0,0));
		bmp->path = assetPlugin(plugin, "res/Blank_New_2.png");
		addChild(bmp);
	}
};

struct Blank_New_3 : ModuleWidget {
	Blank_New_3(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Blank_New_3.svg")));
		BitMap * bmp = Widget::create<BitMap>(Vec(0,0));
		bmp->path = assetPlugin(plugin, "res/Blank_New_3.png");
		addChild(bmp);
	}
};

struct Blank_New_4 : ModuleWidget {
	Blank_New_4(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Blank_New_4.svg")));
		BitMap * bmp = Widget::create<BitMap>(Vec(0,0));
		bmp->path = assetPlugin(plugin, "res/Blank_New_4.png");
		addChild(bmp);
	}
};

#define MODEL(x) Model *modelBlank_##x##HP = Model::create<Module, BlankWidget<x>>("Modular Fungi", "Blank " #x "HP", #x "HP Blanking Plate", BLANK_TAG);
MODEL(3)
MODEL(6)
MODEL(10)
MODEL(16)
MODEL(20)
MODEL(32)
Model *modelBlank_New_1 = Model::create<Module, Blank_New_1>("Modular Fungi", "Blank New_1", "New_1 Blanking Plate", BLANK_TAG);
Model *modelBlank_New_2 = Model::create<Module, Blank_New_2>("Modular Fungi", "Blank New_2", "New_2 Blanking Plate", BLANK_TAG);
Model *modelBlank_New_3 = Model::create<Module, Blank_New_3>("Modular Fungi", "Blank New_3", "New_3 Blanking Plate", BLANK_TAG);
Model *modelBlank_New_4 = Model::create<Module, Blank_New_4>("Modular Fungi", "Blank New_4", "New_4 Blanking Plate", BLANK_TAG);
