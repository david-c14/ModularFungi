#include "OC_Blanks.hpp"

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

struct Blank1 : ModuleWidget {
	Blank1(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Blank1.svg")));
		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
		BitMap * bmp = Widget::create<BitMap>(Vec(0,0));
		bmp->path = assetPlugin(plugin, "res/Blank1.png");
		addChild(bmp);
	}
};

struct Blank2 : ModuleWidget {
	Blank2(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Blank2.svg")));
		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
		BitMap * bmp = Widget::create<BitMap>(Vec(0,0));
		bmp->path = assetPlugin(plugin, "res/Blank2.png");
		addChild(bmp);
	}
};

struct Blank3 : ModuleWidget {
	Blank3(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Blank3.svg")));
		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
		BitMap * bmp = Widget::create<BitMap>(Vec(0,0));
		bmp->path = assetPlugin(plugin, "res/Blank3.png");
		addChild(bmp);
	}
};

struct Blank4 : ModuleWidget {
	Blank4(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Blank4.svg")));
		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
		BitMap * bmp = Widget::create<BitMap>(Vec(0,0));
		bmp->path = assetPlugin(plugin, "res/Blank4.png");
		addChild(bmp);
	}
};

struct Blank5 : ModuleWidget {
	Blank5(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Blank5.svg")));
		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
		BitMap * bmp = Widget::create<BitMap>(Vec(0,0));
		bmp->path = assetPlugin(plugin, "res/Blank5.png");
		addChild(bmp);
	}
};

struct Blank6 : ModuleWidget {
	Blank6(Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Blank6.svg")));
		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
		BitMap * bmp = Widget::create<BitMap>(Vec(0,0));
		bmp->path = assetPlugin(plugin, "res/Blank6.png");
		addChild(bmp);
	}
};

Model *modelBlank1 = Model::create<Module, Blank1>("OC_Blanks", "Blank1", "Blanking Plate Number 1", BLANK_TAG);
Model *modelBlank2 = Model::create<Module, Blank2>("OC_Blanks", "Blank2", "Blanking Plate Number 2", BLANK_TAG);
Model *modelBlank3 = Model::create<Module, Blank3>("OC_Blanks", "Blank3", "Blanking Plate Number 3", BLANK_TAG);
Model *modelBlank4 = Model::create<Module, Blank4>("OC_Blanks", "Blank4", "Blanking Plate Number 4", BLANK_TAG);
Model *modelBlank5 = Model::create<Module, Blank5>("OC_Blanks", "Blank5", "Blanking Plate Number 5", BLANK_TAG);
Model *modelBlank6 = Model::create<Module, Blank6>("OC_Blanks", "Blank6", "Blanking Plate Number 6", BLANK_TAG);
