#include "ModularFungi.hpp"

struct BlankBaseWidget : ModuleWidget {
	int templateSize;
	int selected = 0;
	BitMap *bmp[2];
	std::string FileName(std::string tpl) {
		char workingSpace[100];
		snprintf(workingSpace, 100, tpl.c_str(), templateSize);
		return assetPlugin(plugin, workingSpace);
	}

	BlankBaseWidget(Module *module) : ModuleWidget(module) { }
	void appendContextMenu(Menu *menu) override;
	void setBitmap(int sel) {
		selected = sel;
		bmp[0]->visible = !sel;
		bmp[1]->visible = sel;
	}
	json_t *toJson() override {
		json_t *rootJ = ModuleWidget::toJson();
		json_object_set_new(rootJ, "style", json_real(selected));
		return rootJ;
	}
	void fromJson(json_t *rootJ) override {
		ModuleWidget::fromJson(rootJ);
		json_t *styleJ = json_object_get(rootJ, "style");
		if (styleJ)
			selected = json_number_value(styleJ);
		setBitmap(selected);
	}	
	
};

struct BitmapMenuItem : MenuItem {
	BlankBaseWidget *w;
	int value;
	void onAction(EventAction &e) override {
		w->setBitmap(value);
	}
};

void BlankBaseWidget::appendContextMenu(Menu *menu) {
	menu->addChild(MenuEntry::create());
	BitmapMenuItem *m = MenuItem::create<BitmapMenuItem>("Classic");
	m->w = this;
	m->value = 0;
	m->rightText = CHECKMARK(selected==m->value);
	menu->addChild(m);
	m = MenuItem::create<BitmapMenuItem>("Zen");
	m->w = this;
	m->value = 1;
	m->rightText = CHECKMARK(selected==m->value);
	menu->addChild(m);
}

template<int x>
struct BlankWidget : BlankBaseWidget {
	BlankWidget(Module *module) : BlankBaseWidget(module) {
		templateSize = x;
		box.size = Vec(RACK_GRID_WIDTH * x, RACK_GRID_HEIGHT);
		bmp[0] = Widget::create<BitMap>(Vec(0,0));
		bmp[0]->box.size.x = box.size.x;
		bmp[0]->box.size.y = box.size.y;
		bmp[0]->path = FileName("res/Blank_%dHP.png");
		addChild(bmp[0]);
		bmp[1] = Widget::create<BitMap>(Vec(0,0));
		bmp[1]->box.size.x = box.size.x;
		bmp[1]->box.size.y = box.size.y;
		bmp[1]->path = FileName("res/Zen_%dHP.png");
		bmp[1]->visible = false;
		addChild(bmp[1]);
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
