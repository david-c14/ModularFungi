#include "rack.hpp"
#include "Bitmap.hpp"

MFTextureList gTextureList;

void BitMap::DrawImage(NVGcontext *vg) {
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
void BitMap::draw(NVGcontext *vg) {
	DrawImage(vg);
	TransparentWidget::draw(vg);
}
