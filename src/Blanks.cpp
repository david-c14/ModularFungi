#include "ModularFungi.hpp"

struct BlankBaseWidget : ModuleWidget {
	static constexpr int LISTSIZE = 2;
	int selected = 0;
	std::string fileName[LISTSIZE];
	BitMap *bmp;
	std::string FileName(std::string tpl, int templateSize) {
		char workingSpace[100];
		snprintf(workingSpace, 100, tpl.c_str(), templateSize);
		return assetPlugin(pluginInstance, workingSpace);
	}

	BlankBaseWidget(Module *module) : ModuleWidget() {
		setModule(module);
	}
	void appendContextMenu(Menu *menu) override;
	void loadBitmap() {
		bmp = createWidget<BitMap>(Vec(0,0));
		bmp->box.size.x = box.size.x;
		bmp->box.size.y = box.size.y;
		bmp->path = fileName[selected];
		addChild(bmp);
	}
	void setBitmap(int sel) {
		if (selected == sel)
			return;
		selected = clamp(sel, 0, LISTSIZE - 1);
		removeChild(bmp);
		delete bmp;
		loadBitmap();
	}
	json_t *dataToJson() override {
		json_t *rootJ = ModuleWidget::dataToJson();
		json_object_set_new(rootJ, "style", json_real(selected));
		return rootJ;
	}
	void dataFromJson(json_t *rootJ) override {
		ModuleWidget::dataFromJson(rootJ);
		int sel = selected;
		json_t *styleJ = json_object_get(rootJ, "style");
		if (styleJ)
			sel = json_number_value(styleJ);
		setBitmap(sel);
	}	
	
};

struct BitmapMenuItem : MenuItem {
	BlankBaseWidget *w;
	int value;
	void onAction(const event::Action &e) override {
		w->setBitmap(value);
	}
};

void BlankBaseWidget::appendContextMenu(Menu *menu) {
	menu->addChild(new MenuEntry);
	BitmapMenuItem *m = createMenuItem<BitmapMenuItem>("Classic");
	m->w = this;
	m->value = 0;
	m->rightText = CHECKMARK(selected==m->value);
	menu->addChild(m);
	m = createMenuItem<BitmapMenuItem>("Zen");
	m->w = this;
	m->value = 1;
	m->rightText = CHECKMARK(selected==m->value);
	menu->addChild(m);
}

template<int x>
struct BlankWidget : BlankBaseWidget {
	BlankWidget(Module *module) : BlankBaseWidget(module) {
		fileName[0] = FileName("res/Blank_%dHP.png", x);
		fileName[1] = FileName("res/Zen_%dHP.png", x);
		box.size = Vec(RACK_GRID_WIDTH * x, RACK_GRID_HEIGHT);
		loadBitmap();
	}
};

Model *modelBlank_1HP = createModel<Module, BlankWidget<1>>("Blank 1HP");
Model *modelBlank_3HP = createModel<Module, BlankWidget<3>>("Blank 3HP");
Model *modelBlank_4HP = createModel<Module, BlankWidget<4>>("Blank 4HP");
Model *modelBlank_6HP = createModel<Module, BlankWidget<6>>("Blank 6HP");
Model *modelBlank_10HP = createModel<Module, BlankWidget<10>>("Blank 10HP");
Model *modelBlank_12HP = createModel<Module, BlankWidget<12>>("Blank 12HP");
Model *modelBlank_16HP = createModel<Module, BlankWidget<16>>("Blank 16HP");
Model *modelBlank_20HP = createModel<Module, BlankWidget<20>>("Blank 20HP");
Model *modelBlank_26HP = createModel<Module, BlankWidget<26>>("Blank 26HP");
Model *modelBlank_32HP = createModel<Module, BlankWidget<32>>("Blank 32HP");
