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

struct Blank_3HP : ModuleWidget {
	Blank_3HP(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Blank_3HP.svg")));
		BitMap * bmp = Widget::create<BitMap>(Vec(0,0));
		bmp->path = assetPlugin(plugin, "res/Blank_3HP.png");
		addChild(bmp);
	}
};

struct Blank_6HP : ModuleWidget {
	Blank_6HP(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Blank_6HP.svg")));
		BitMap * bmp = Widget::create<BitMap>(Vec(0,0));
		bmp->path = assetPlugin(plugin, "res/Blank_6HP.png");
		addChild(bmp);
	}
};

struct Blank_10HP : ModuleWidget {
	Blank_10HP(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Blank_10HP.svg")));
		BitMap * bmp = Widget::create<BitMap>(Vec(0,0));
		bmp->path = assetPlugin(plugin, "res/Blank_10HP.png");
		addChild(bmp);
	}
};

struct Blank_16HP : ModuleWidget {
	Blank_16HP(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Blank_16HP.svg")));
		BitMap * bmp = Widget::create<BitMap>(Vec(0,0));
		bmp->path = assetPlugin(plugin, "res/Blank_16HP.png");
		addChild(bmp);
	}
};

struct Blank_20HP : ModuleWidget {
	Blank_20HP(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Blank_20HP.svg")));
		BitMap * bmp = Widget::create<BitMap>(Vec(0,0));
		bmp->path = assetPlugin(plugin, "res/Blank_20HP.png");
		addChild(bmp);
	}
};

struct Blank_32HP : ModuleWidget {
	Blank_32HP(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Blank_32HP.svg")));
		BitMap * bmp = Widget::create<BitMap>(Vec(0,0));
		bmp->path = assetPlugin(plugin, "res/Blank_32HP.png");
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


Model *modelBlank_3HP = Model::create<Module, Blank_3HP>("Modular Fungi", "Blank 3HP", "3HP Blanking Plate", BLANK_TAG);
Model *modelBlank_6HP = Model::create<Module, Blank_6HP>("Modular Fungi", "Blank 6HP", "6HP Blanking Plate", BLANK_TAG);
Model *modelBlank_10HP = Model::create<Module, Blank_10HP>("Modular Fungi", "Blank 10HP", "10HP Blanking Plate", BLANK_TAG);
Model *modelBlank_16HP = Model::create<Module, Blank_16HP>("Modular Fungi", "Blank 16HP", "16HP Blanking Plate", BLANK_TAG);
Model *modelBlank_20HP = Model::create<Module, Blank_20HP>("Modular Fungi", "Blank 20HP", "20HP Blanking Plate", BLANK_TAG);
Model *modelBlank_32HP = Model::create<Module, Blank_32HP>("Modular Fungi", "Blank 32HP", "32HP Blanking Plate", BLANK_TAG);
Model *modelBlank_New_1 = Model::create<Module, Blank_New_1>("Modular Fungi", "Blank New_1", "New_1 Blanking Plate", BLANK_TAG);
Model *modelBlank_New_2 = Model::create<Module, Blank_New_2>("Modular Fungi", "Blank New_2", "New_2 Blanking Plate", BLANK_TAG);
Model *modelBlank_New_3 = Model::create<Module, Blank_New_3>("Modular Fungi", "Blank New_3", "New_3 Blanking Plate", BLANK_TAG);
Model *modelBlank_New_4 = Model::create<Module, Blank_New_4>("Modular Fungi", "Blank New_4", "New_4 Blanking Plate", BLANK_TAG);
