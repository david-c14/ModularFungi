#pragma once

#include "rack.hpp"
#include "Texture.hpp"

using namespace rack;

struct BitMap : TransparentWidget {
	std::string path;
	int loaded = false;
	std::shared_ptr<MFTexture> bitmap;
	NVGcontext *storedVG;
	void DrawImage(NVGcontext *vg);
	void draw(NVGcontext *vg) override;
	~BitMap() {
		if (bitmap)
			bitmap->release();
	}
};
