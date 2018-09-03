#include "ModularFungi.hpp"

struct MFTexture {
	int image;
	std::string name;
	NVGcontext *context;
	int width;
	int height;
	int refCount = 0;
	MFTexture(NVGcontext *vg, std::string fileName, int imageFlags) {
		reload(vg, fileName, imageFlags);
	}
	void reload(NVGcontext *vg, std::string fileName, int imageFlags) {
		if (image)
			nvgDeleteImage(vg, image);
		image = nvgCreateImage(vg, fileName.c_str(), imageFlags);
		name = fileName;
		context = vg;
		refCount++;
		if (!image)
			return;
		nvgImageSize(vg, image, &width, &height);
	}
	void release() {
		refCount--;
		if (refCount)
			return;
		refCount = 0;
		if (image)
			nvgDeleteImage(context, image);
		image = 0;
	}
	~MFTexture() {
		release();
	}
};

struct MFTextureList {
	std::vector<std::shared_ptr<MFTexture>> list;
	std::shared_ptr<MFTexture> load(NVGcontext *vg, std::string fileName, int imageFlags) {
		for (std::shared_ptr<MFTexture> tex : list) {
			if ((tex->context == vg) && !tex->name.compare(fileName)) {
				if (tex->image) {
					tex->refCount++;
					return tex;
				}
				tex->reload(vg, fileName, imageFlags);	
				return tex;
			}
		}
		std::shared_ptr<MFTexture> tex = std::make_shared<MFTexture>(vg, fileName, imageFlags);
		list.push_back(tex);
		return tex;
	}
};

MFTextureList gTextureList;

struct BitMap : TransparentWidget {
	std::string path;
	int loaded = false;
	std::shared_ptr<MFTexture> bitmap;
	NVGcontext *storedVG;
	void DrawImage(NVGcontext *vg) {
		storedVG = vg;	
		if (!loaded) {
			loaded = true;
			bitmap = gTextureList.load(vg, path, NVG_IMAGE_GENERATE_MIPMAPS);
			if (!bitmap->image)
				warn("ModularFungi: Unable to load %s", path.c_str());
		}
		if (!bitmap->image)
			return;	
		NVGpaint paint = nvgImagePattern(vg, 0, 0, box.size.x, box.size.y, 0.0f, bitmap->image, 1.0f);
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
		if (bitmap)
			bitmap->release();
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
