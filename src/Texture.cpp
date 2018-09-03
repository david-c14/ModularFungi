#include "Texture.hpp"

void MFTexture::reload(NVGcontext *vg, std::string fileName, int imageFlags) {
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
void MFTexture::release() {
	refCount--;
	if (refCount)
		return;
	refCount = 0;
	if (image)
		nvgDeleteImage(context, image);
	image = 0;
}

std::shared_ptr<MFTexture> MFTextureList::load(NVGcontext *vg, std::string fileName, int imageFlags) {
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
