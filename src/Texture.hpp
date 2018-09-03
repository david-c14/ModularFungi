#pragma once

#include "rack.hpp"

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
	void reload(NVGcontext *vg, std::string fileName, int imageFlags);
	void release();
	~MFTexture() {
		release();
	}
};

struct MFTextureList {
	std::vector<std::shared_ptr<MFTexture>> list;
	std::shared_ptr<MFTexture> load(NVGcontext *vg, std::string fileName, int imageFlags);
};
