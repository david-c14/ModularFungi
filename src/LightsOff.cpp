// "Lights Off" module for VCV Rack
//
// BSD 3-Clause License
//
// Copyright (C) 2020 Benjamin Dill
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "ModularFungi.hpp"


struct LightsOffModule : Module {
	enum ParamIds {
		PARAM_DIM,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		LIGHT_ENABLED,
		NUM_LIGHTS
	};

	bool active = false;

	LightsOffModule() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PARAM_DIM, 0.0f, 1.0f, 0.8f, "Dim", "%", 0.f, 100.f);
	}
};

static LightsOffModule *lightsOffSingleton = NULL;


struct LightsOffContainer : widget::Widget {
	LightsOffModule *module;

	void draw(const DrawArgs& args) override {
		if (module && module->active) {
			// Dim layer
			box = parent->box.zeroPos();
			nvgBeginPath(args.vg);
			nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
			nvgFillColor(args.vg, nvgRGBA(0x00, 0x00, 0x00, (char)(255.f * module->params[LightsOffModule::PARAM_DIM].getValue())));
			nvgFill(args.vg);

			// Draw lights
			Rect viewPort = getViewport(box);
			std::queue<Widget*> q;
			q.push(APP->scene->rack->moduleContainer);
			while (!q.empty()) {
				Widget* w = q.front();
				q.pop();

				LightWidget *lw = dynamic_cast<LightWidget*>(w);
				if (lw) {
					Vec p1 = lw->getRelativeOffset(Vec(), this);
					Vec p = getAbsoluteOffset(Vec()).neg();
					p = p.plus(p1);
					p = p.div(APP->scene->rackScroll->zoomWidget->zoom);

					// Draw only if currently visible
					if (viewPort.isIntersecting(Rect(p, lw->box.size))) {
						nvgSave(args.vg);
						nvgResetScissor(args.vg);
						nvgTranslate(args.vg, p.x, p.y);
						lw->draw(args);
						nvgRestore(args.vg);
					}
				}

				for (Widget *w1 : w->children) {
					q.push(w1);
				}
			}

			// Draw cable plugs
			for (widget::Widget *w : APP->scene->rack->cableContainer->children) {
				CableWidget *cw = dynamic_cast<CableWidget*>(w);
				assert(cw);
				cw->drawPlugs(args);
			}
		}
		Widget::draw(args);
	}
	
	void onHoverKey(const event::HoverKey &e) override {
		const char* keyName = glfwGetKeyName(e.key, 0);
		if (e.action == GLFW_PRESS && keyName && *keyName == 'x' && (e.mods & RACK_MOD_MASK) == (RACK_MOD_CTRL | GLFW_MOD_ALT)) {
			module->active ^= true;
		}
		Widget::onHoverKey(e);
	}
};

struct LightsOffWidget : ModuleWidget {
	BitMap *bmp;
	LightsOffContainer *loContainer;
	bool enabled = false;

	LightsOffWidget(LightsOffModule *module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		bmp = createWidget<BitMap>(Vec(0,0));
		bmp->box.size.x = box.size.x;
		bmp->box.size.y = box.size.y;
		bmp->path = FileName("res/LightsOff.png", 1);
		addChild(bmp);

		addChild(createLightCentered<TinyLight<WhiteLight>>(Vec(7.5f, 38.0f), module, LightsOffModule::LIGHT_ENABLED));

		if (module && !lightsOffSingleton) {
			enabled = true;
			lightsOffSingleton = module;
			if (enabled) {
				loContainer = new LightsOffContainer;
				loContainer->module = module;
				// This is where the magic happens: add a new widget on top-level to Rack
				APP->scene->rack->addChild(loContainer);
			}
		}
	}

	~LightsOffWidget() {
		if (enabled && loContainer) {
			lightsOffSingleton = NULL;
			APP->scene->rack->removeChild(loContainer);
			delete loContainer;
		}
	}

	std::string FileName(std::string tpl, int templateSize) {
		char workingSpace[100];
		snprintf(workingSpace, 100, tpl.c_str(), templateSize);
		return asset::plugin(pluginInstance, workingSpace);
	}

	void step() override {
		if (module) {
			module->lights[LightsOffModule::LIGHT_ENABLED].setBrightness(enabled);
		}
		ModuleWidget::step();
	}

	void appendContextMenu(Menu *menu) override {
		LightsOffModule *module = dynamic_cast<LightsOffModule*>(this->module);

		struct ActiveItem : MenuItem {
			LightsOffModule *module;
			void onAction(const event::Action &e) override {
				module->active ^= true;
			}
			void step() override {
				rightText = module->active ? "✔" : "";
				MenuItem::step();
			}
		};

		struct DimSlider : ui::Slider {
			DimSlider(LightsOffModule *module) {
				box.size.x = 180.0f;
				quantity = module->paramQuantities[LightsOffModule::PARAM_DIM];
			}
		};

		menu->addChild(new MenuSeparator());
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Hotkey " RACK_MOD_CTRL_NAME "+Alt+X"));
		menu->addChild(construct<ActiveItem>(&MenuItem::text, "Active", &ActiveItem::module, module));
		menu->addChild(new DimSlider(module));
	}
};

Model *modelLightsOff = createModel<LightsOffModule, LightsOffWidget>("LightsOff");
