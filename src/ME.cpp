#include "ModularFungi.hpp"

struct MEBitMap : TransparentWidget {
	std::string path;
	int loaded = false;
	int bitmap = 0;
	NVGcontext *storedVG;
	void DrawImage(NVGcontext *vg) {
		storedVG = vg;	
		if (!loaded) {
			loaded = true;
			bitmap = nvgCreateImage(vg, path.c_str(), NVG_IMAGE_GENERATE_MIPMAPS | NVG_IMAGE_REPEATX);
			if (!bitmap)
				warn("ModularFungi: Unable to load %s", path.c_str());
		}
		if (!bitmap)
			return;	
		NVGpaint paint = nvgImagePattern(vg, 0, 0, 60, box.size.y, 0.0f, bitmap, 1.0f);
		nvgSave(vg);
		nvgBeginPath(vg);
		nvgRect(vg, 0, 0, box.size.x, box.size.y);
		nvgFillColor(vg, nvgRGB(0, 0, 0));
		nvgFill(vg);

		nvgBeginPath(vg);
		nvgRect(vg, 10, 10, 100, 100);
		nvgFillColor(vg, nvgRGB(0xff, 0xff, 0xff));
		nvgFill(vg);
		nvgGlobalCompositeBlendFunc(vg, NVG_DST_COLOR, NVG_ZERO);

		nvgFillPaint(vg, paint);
		nvgBeginPath(vg);
		nvgRect(vg, 0, 0, box.size.x, box.size.y);
		nvgFill(vg);
		nvgRestore(vg);
		
	}
	void draw(NVGcontext *vg) override {
		DrawImage(vg);
		TransparentWidget::draw(vg);
	}
	~MEBitMap() {
		if (!bitmap)
			nvgDeleteImage(storedVG, bitmap);
	}
};

struct MEWidget : ModuleWidget {

	MEWidget(Module *module) : ModuleWidget(module) {
		box.size = Vec(RACK_GRID_WIDTH * 20, RACK_GRID_HEIGHT);
		MEBitMap * bmp = Widget::create<MEBitMap>(Vec(0,0));
		bmp->box.size.x = box.size.x;
		bmp->box.size.y = box.size.y;
		bmp->path = assetPlugin(plugin, "res/Blank_4HP.png");
		addChild(bmp);
	}
};

Model *modelME = Model::create<Module, MEWidget>("Modular Fungi", "Test", "Test Blanking Place", BLANK_TAG);
