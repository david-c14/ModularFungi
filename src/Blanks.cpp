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
		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
		BitMap * bmp = Widget::create<BitMap>(Vec(0,0));
		bmp->path = assetPlugin(plugin, "res/Blank_3HP.png");
		addChild(bmp);
	}
};

struct Blank_6HP : ModuleWidget {
	Blank_6HP(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Blank_6HP.svg")));
		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
		BitMap * bmp = Widget::create<BitMap>(Vec(0,0));
		bmp->path = assetPlugin(plugin, "res/Blank_6HP.png");
		addChild(bmp);
	}
};

struct Blank_10HP : ModuleWidget {
	Blank_10HP(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Blank_10HP.svg")));
		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
		BitMap * bmp = Widget::create<BitMap>(Vec(0,0));
		bmp->path = assetPlugin(plugin, "res/Blank_10HP.png");
		addChild(bmp);
	}
};

struct Blank_16HP : ModuleWidget {
	Blank_16HP(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Blank_16HP.svg")));
		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
		BitMap * bmp = Widget::create<BitMap>(Vec(0,0));
		bmp->path = assetPlugin(plugin, "res/Blank_16HP.png");
		addChild(bmp);
	}
};

struct Blank_20HP : ModuleWidget {
	Blank_20HP(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Blank_20HP.svg")));
		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
		BitMap * bmp = Widget::create<BitMap>(Vec(0,0));
		bmp->path = assetPlugin(plugin, "res/Blank_20HP.png");
		addChild(bmp);
	}
};

struct Blank_32HP : ModuleWidget {
	Blank_32HP(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Blank_32HP.svg")));
		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
		BitMap * bmp = Widget::create<BitMap>(Vec(0,0));
		bmp->path = assetPlugin(plugin, "res/Blank_32HP.png");
		addChild(bmp);
	}
};

Model *modelBlank_3HP = Model::create<Module, Blank_3HP>("ModularFungi", "Blank_3HP", "3HP Blanking Plate", BLANK_TAG);
Model *modelBlank_6HP = Model::create<Module, Blank_6HP>("ModularFungi", "Blank_6HP", "6HP Blanking Plate", BLANK_TAG);
Model *modelBlank_10HP = Model::create<Module, Blank_10HP>("ModularFungi", "Blank_10HP", "10HP Blanking Plate", BLANK_TAG);
Model *modelBlank_16HP = Model::create<Module, Blank_16HP>("ModularFungi", "Blank_16HP", "16HP Blanking Plate", BLANK_TAG);
Model *modelBlank_20HP = Model::create<Module, Blank_20HP>("ModularFungi", "Blank_20HP", "20HP Blanking Plate", BLANK_TAG);
Model *modelBlank_32HP = Model::create<Module, Blank_32HP>("ModularFungi", "Blank_32HP", "32HP Blanking Plate", BLANK_TAG);
