#include <string.h>
#include <memory>
#include <atomic>
#include "ModularFungi.hpp"

//All **source code** is copyright © 2019 Andrew Belt and is licensed under the [GNU General Public License v3.0](LICENSE-GPLv3.txt).

//Modified by Dave French 2020
//Added use with lights off module, kaleidoscope plots, resizable, line types, line widths & fading lines
//reworked UI, moved options to context menu.

static const int BUFFER_SIZE = 4096;

struct Scope : Module {
	enum ParamIds {
		X_SCALE_PARAM,
		X_POS_PARAM,
		Y_SCALE_PARAM,
		Y_POS_PARAM,
		TIME_PARAM,
		LISSAJOUS_PARAM,
		TRIG_PARAM,
		EXTERNAL_PARAM,
		KALEIDOSCOPE_USE_PARAM,
		KALEIDOSCOPE_COUNT_PARAM,
		KALEIDOSCOPE_RADIUS_PARAM,
		LINE_WIDTH_PARAM,
		LINE_FADE_PARAM,
		LINE_COLOR_PARAM,
		LINE_TYPE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		X_INPUT,
		Y_INPUT,
		TRIG_INPUT,
		COLOR_INPUT,
		LINE_WIDTH_INPUT,
		KALEIDOSCOPE_COUNT_INPUT,
		KALEIDOSCOPE_RADIUS_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	enum LineType {
		NORMAL_LINE,
		EXPERIMENTAL_LINE,
		VECTOR_LINE,
		NUM_LINES
	} lineType;

	float bufferX[16][BUFFER_SIZE] = {};
	float bufferY[16][BUFFER_SIZE] = {};
	int channelsX = 0;
	int channelsY = 0;
	int bufferIndex = 0;
	int frameIndex = 0;

	//parameters for kaleidoscope
	struct Kaleidoscope {
		bool use = false;
		int count = 3;
		float radius = 20.0f;
	} kaleidoscope;

	dsp::BooleanTrigger sumTrigger;
	dsp::BooleanTrigger extTrigger;
	dsp::BooleanTrigger kaleidoscopeTrigger;
	bool lissajous = false;
	bool external = false;
	dsp::SchmittTrigger triggers[16];
	float hue = 0.5f;
	float lineWidth = 1.5f;
	float fade = 1.0f;
	std::atomic<float> widgetWidth;

	Scope() {
		widgetWidth.store(RACK_GRID_WIDTH * 20);

		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(X_SCALE_PARAM, -2.f, 8.f, 0.f, "X scale", " V/div", 1 / 2.f, 5);
		configParam(X_POS_PARAM, -10.f, 10.f, 0.f, "X position", " V");
		configParam(Y_SCALE_PARAM, -2.f, 8.f, 0.f, "Y scale", " V/div", 1 / 2.f, 5);
		configParam(Y_POS_PARAM, -10.f, 10.f, 0.f, "Y position", " V");
		const float timeBase = (float) BUFFER_SIZE / 6;
		configParam(TIME_PARAM, 6.f, 16.f, 14.f, "Time", " ms/div", 1 / 2.f, 1000 * timeBase);
		configParam(LISSAJOUS_PARAM, 0.f, 1.f, 0.f, "X & Y / Lissajous");
		configParam(TRIG_PARAM, -10.f, 10.f, 0.f, "Trigger position", " V");
		configParam(EXTERNAL_PARAM, 0.f, 1.f, 0.f, "Internal / External Trigger");

		configParam(KALEIDOSCOPE_USE_PARAM, 0.0f, 1.0f, 0.0f, "Kaleidoscope");
		configParam(KALEIDOSCOPE_COUNT_PARAM, 3.0f, 12.0f, 3.0f, "Mirrors");
		configParam(KALEIDOSCOPE_RADIUS_PARAM, 1.0f, 200.0f, 1.0f, "Radius");

		configParam(LINE_TYPE_PARAM, 0.0f, NUM_LINES - 1, Scope::LineType::NORMAL_LINE, "Line Type");
		configParam(LINE_WIDTH_PARAM, 0.1f, 10.0f, 1.5f, "Line Width");
		configParam(LINE_COLOR_PARAM, 0.0f, 1.0f, 5.0f, "Color");
		configParam(LINE_FADE_PARAM, 0.0f, 1.0f, 1.0f, "Fade");
	}

	void onReset() override {
		lissajous = false;
		external = false;
		kaleidoscope.use = false;
		std::memset(bufferX, 0, sizeof(bufferX));
		std::memset(bufferY, 0, sizeof(bufferY));
	}

	void process(const ProcessArgs &args) override {
		//kaleidoscope parameters
		kaleidoscope.count = (int) clamp(
				params[KALEIDOSCOPE_COUNT_PARAM].getValue() + inputs[KALEIDOSCOPE_COUNT_INPUT].getVoltage(), 3.0f,
				12.0f);
		kaleidoscope.radius =
				params[KALEIDOSCOPE_RADIUS_PARAM].getValue() + inputs[KALEIDOSCOPE_RADIUS_INPUT].getVoltage() * 10;

		hue = params[LINE_COLOR_PARAM].getValue() + inputs[COLOR_INPUT].getVoltage() / 10.0f;
		lineWidth = params[LINE_WIDTH_PARAM].getValue() + inputs[LINE_WIDTH_INPUT].getVoltage();
		fade = params[LINE_FADE_PARAM].getValue();
		lineType = (Scope::LineType) params[LINE_TYPE_PARAM].getValue();

		// Modes
		if (sumTrigger.process(params[LISSAJOUS_PARAM].getValue() > 0.f)) {
			lissajous = !lissajous;
		}

		if (extTrigger.process(params[EXTERNAL_PARAM].getValue() > 0.f)) {
			external = !external;
		}

		if (kaleidoscopeTrigger.process(params[KALEIDOSCOPE_USE_PARAM].getValue() > 0.0f))
			kaleidoscope.use = !kaleidoscope.use;

		// Compute time
		float deltaTime = std::pow(2.f, -params[TIME_PARAM].getValue());
		int frameCount = (int) std::ceil(deltaTime * args.sampleRate);

		// Set channels
		int channelsX = inputs[X_INPUT].getChannels();
		if (channelsX != this->channelsX) {
			std::memset(bufferX, 0, sizeof(bufferX));
			this->channelsX = channelsX;
		}

		int channelsY = inputs[Y_INPUT].getChannels();
		if (channelsY != this->channelsY) {
			std::memset(bufferY, 0, sizeof(bufferY));
			this->channelsY = channelsY;
		}

		// Add frame to buffer
		if (bufferIndex < BUFFER_SIZE) {
			if (++frameIndex > frameCount) {
				frameIndex = 0;
				for (int c = 0; c < channelsX; c++) {
					bufferX[c][bufferIndex] = inputs[X_INPUT].getVoltage(c);
				}
				for (int c = 0; c < channelsY; c++) {
					bufferY[c][bufferIndex] = inputs[Y_INPUT].getVoltage(c);
				}
				bufferIndex++;
			}
		}

		// Don't wait for trigger if still filling buffer
		if (bufferIndex < BUFFER_SIZE) {
			return;
		}

		// Trigger immediately if external but nothing plugged in, or in Lissajous mode
		if (lissajous || (external && !inputs[TRIG_INPUT].isConnected())) {
			trigger();
			return;
		}

		frameIndex++;

		// Reset if triggered
		float trigThreshold = params[TRIG_PARAM].getValue();
		Input &trigInput = external ? inputs[TRIG_INPUT] : inputs[X_INPUT];

		// This may be 0
		int trigChannels = trigInput.getChannels();
		for (int c = 0; c < trigChannels; c++) {
			float trigVoltage = trigInput.getVoltage(c);
			if (triggers[c].process(rescale(trigVoltage, trigThreshold, trigThreshold + 0.001f, 0.f, 1.f))) {
				trigger();
				return;
			}
		}

		// Reset if we've been waiting for `holdTime`
		const float holdTime = 0.1f;
		if (frameIndex * args.sampleTime >= holdTime) {
			trigger();
			return;
		}
	}

	void trigger() {
		for (int c = 0; c < 16; c++) {
			triggers[c].reset();
		}
		bufferIndex = 0;
		frameIndex = 0;
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "lissajous", json_integer((int) lissajous));
		json_object_set_new(rootJ, "external", json_integer((int) external));
		json_object_set_new(rootJ, "kaleidoscope", json_integer((int) kaleidoscope.use));
		json_object_set_new(rootJ, "WidgetWidth", json_real(widgetWidth.load()));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t *sumJ = json_object_get(rootJ, "lissajous");
		if (sumJ)
			lissajous = json_integer_value(sumJ);

		json_t *extJ = json_object_get(rootJ, "external");
		if (extJ)
			external = json_integer_value(extJ);

		json_t *useK = json_object_get(rootJ, "kaleidoscope");
		if (useK)
			kaleidoscope.use = (bool) json_integer_value(useK);

		json_t *ww = json_object_get(rootJ, "WidgetWidth");
		if (ww)
			widgetWidth.store((float) json_real_value(ww));
	}
};

// USER INTERFACE  ****************

/// Placed on right of module, allow resizing of parent widget via drag n drop
struct ResizeTab : OpaqueWidget {
	Vec position = {};
	Rect oldBounds = {};

	ResizeTab() {
		box.size = Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	}

	void onDragStart(const event::DragStart &e) override {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
			//save start size
			auto *modWidget = getAncestorOfType<ModuleWidget>();
			assert(modWidget);
			if (modWidget != nullptr)
				oldBounds = modWidget->box;

			// mousedown position
			position = APP->scene->rack->mousePos;
		}
	}

	void onDragMove(const event::DragMove &e) override {
		auto *modWidget = getAncestorOfType<ModuleWidget>();
		assert(modWidget);
		if (modWidget == nullptr)
			return;

		auto newPosition = APP->scene->rack->mousePos;
		auto xChange = newPosition.x - position.x;
		auto newRect = oldBounds;
		auto oldRect = modWidget->box;
		auto minW = 10 * RACK_GRID_WIDTH;

		newRect.size.x += xChange;
		newRect.size.x = std::max(newRect.size.x, minW);
		newRect.size.x = std::round(newRect.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;

		modWidget->box = newRect;
		if (!APP->scene->rack->requestModulePos(modWidget, newRect.pos))
			modWidget->box = oldRect;
	}
};

struct ScopePanel : Widget {
	Widget *panelBorder = nullptr;
	NVGcolor backGroundColor;

	ScopePanel(NVGcolor color) {
		panelBorder = new PanelBorder;
		backGroundColor = color;
		addChild(panelBorder);
	}

	void step() override {
		panelBorder->box.size = box.size;
		Widget::step();
	}

	void draw(const DrawArgs &args) override {
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
		nvgFillColor(args.vg, backGroundColor);
		nvgFill(args.vg);
		Widget::draw(args);
	}
};

struct ScopeDisplay : ModuleLightWidget {
	Scope *module;
	int statsFrame = 0;
	std::shared_ptr<Font> font;

	struct Stats {
		float vpp = 0.f;
		float vmin = 0.f;
		float vmax = 0.f;

		void calculate(float *buffer, int channels) {
			vmax = -INFINITY;
			vmin = INFINITY;
			for (int i = 0; i < BUFFER_SIZE * channels; i++) {
				float v = buffer[i];
				vmax = std::fmax(vmax, v);
				vmin = std::fmin(vmin, v);
			}
			vpp = vmax - vmin;
		}
	};

	Stats statsX, statsY;

	ScopeDisplay() {
		font = APP->window->loadFont(asset::system("res/fonts/ShareTechMono-Regular.ttf"));
	}

	void drawWaveform(const DrawArgs &args,
								  const float *bufferX,
								  float offsetX,
								  float gainX,
								  const float *bufferY,
								  float offsetY,
								  float gainY,
								  float kRadius = 0.0f,
								  float kRotation = 0.0f,
								  NVGcolor beam = {1.0f, 1.0f, 1.0f, 1.0f}) {
		assert(bufferY);
		nvgSave(args.vg);

		//beam fading using a varying alpha
		auto maxAlpha = 0.99f;
		auto lightInc = maxAlpha / BUFFER_SIZE;
		auto currentAlpha = maxAlpha;

		auto currentLineWidth = module->lineWidth;
		auto widthInc = module->lineWidth / BUFFER_SIZE;

		Rect b = Rect(Vec(0, 15), box.size.minus(Vec(0, 15 * 2)));
		nvgBeginPath(args.vg);
		nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
		nvgTranslate(args.vg, kRadius * simd::cos(kRotation) + box.size.x / 2.0f,
					 kRadius * simd::sin(kRotation) - (box.size.y - 30) / 2.0f);
		if (!module->lissajous)
			nvgTranslate(args.vg, -box.size.x / 2.0f, 0);

		nvgLineCap(args.vg, NVG_BUTT);
		nvgMiterLimit(args.vg, 2.f);
		nvgStrokeWidth(args.vg, module->lineWidth);
		nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);

		auto cos2R = simd::cos(2.0f * kRotation);
		auto sin2R = simd::sin(2.0f * kRotation);
		//start drawing for current buffer write position
		//and loop back.
		assert(module);
		for (int i = module->bufferIndex - 1; i != module->bufferIndex; i--) {
			if (i < 0)
				i = BUFFER_SIZE - 1; // loop buffer due to starting at various locations

			nvgStrokeColor(args.vg, nvgRGBAf(beam.r, beam.g, beam.b, currentAlpha));
			nvgStrokeWidth(args.vg, currentLineWidth);
			if ((bool) module->fade) {
				currentAlpha -= lightInc;
				currentLineWidth -= widthInc;
			}
			Vec v;
			if (bufferX) {
				v.x = (bufferX[i] + offsetX) * gainX / 2.0f;
			} else {
				v.x = (float) i / (BUFFER_SIZE - 1);
			}
			v.y = (bufferY[i] + offsetY) * gainY / 2.0f;

			//rotate 2 * kRotate

			auto xNew = v.x * cos2R + v.y * sin2R;
			v.y = -v.x * sin2R + v.y * cos2R;
			v.x = xNew;

			Vec p;
			// resize x & y by height, to keep plots in correct ratio if Lissajous
			if (module->lissajous)
				p.x = rescale(v.x, 0.f, 1.f, b.pos.x, b.pos.y + b.size.y);
			else
				p.x = rescale(v.x, 0.f, 1.f, b.pos.x, b.pos.x + b.size.x);

			p.y = rescale(v.y, 0.f, 1.f, b.pos.y + b.size.y, b.pos.y);
			if (i == BUFFER_SIZE - 1) {
				nvgMoveTo(args.vg, p.x, p.y);
			} else {
				auto vectorScale = 0.998f;
				auto dotScale = 0.9f;
				switch (module->lineType) {
					case Scope::LineType::NORMAL_LINE:
						nvgLineTo(args.vg, p.x, p.y);
						break;
					case Scope::LineType::VECTOR_LINE:
						nvgMoveTo(args.vg, vectorScale * p.x, vectorScale * p.y);
						nvgLineTo(args.vg, p.x, p.y);
						break;
					case Scope::LineType::EXPERIMENTAL_LINE:
						nvgMoveTo(args.vg, dotScale * p.x, dotScale * p.y);
						nvgLineTo(args.vg, p.x, p.y);
						break;
					case Scope::NUM_LINES:
						assert(false);
						break;
					default:
						assert(false);
				}
			}
			nvgStroke(args.vg);
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, p.x, p.y);
		}
		nvgResetTransform(args.vg);
		nvgResetScissor(args.vg);
		nvgRestore(args.vg);
	}

	void drawTrig(const DrawArgs &args, float value) {
		Rect b = Rect(Vec(0, 15), box.size.minus(Vec(0, 15 * 2)));
		nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);

		value = value / 2.f + 0.5f;
		Vec p = Vec(box.size.x, b.pos.y + b.size.y * (1.f - value));

		// Draw line
		nvgStrokeColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0x10));
		{
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, p.x - 13, p.y);
			nvgLineTo(args.vg, 0, p.y);
			nvgClosePath(args.vg);
		}
		nvgStroke(args.vg);

		// Draw indicator
		nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0x60));
		{
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, p.x - 2, p.y - 4);
			nvgLineTo(args.vg, p.x - 9, p.y - 4);
			nvgLineTo(args.vg, p.x - 13, p.y);
			nvgLineTo(args.vg, p.x - 9, p.y + 4);
			nvgLineTo(args.vg, p.x - 2, p.y + 4);
			nvgClosePath(args.vg);
		}
		nvgFill(args.vg);

		nvgFontSize(args.vg, 9);
		nvgFontFaceId(args.vg, font->handle);
		nvgFillColor(args.vg, nvgRGBA(0x1e, 0x28, 0x2b, 0xff));
		nvgText(args.vg, p.x - 8, p.y + 3, "T", NULL);
		nvgResetScissor(args.vg);
	}

	void drawStats(const DrawArgs &args, Vec pos, const char *title, Stats *stats) {
		nvgFontSize(args.vg, 13);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, -2);

		nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0x40));
		nvgText(args.vg, pos.x + 6, pos.y + 11, title, NULL);

		nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0x80));
		pos = pos.plus(Vec(22, 11));

		std::string text;
		text = "pp ";
		text += isNear(stats->vpp, 0.f, 100.f) ? string::f("% 6.2f", stats->vpp) : "  ---";
		nvgText(args.vg, pos.x, pos.y, text.c_str(), NULL);
		text = "max ";
		text += isNear(stats->vmax, 0.f, 100.f) ? string::f("% 6.2f", stats->vmax) : "  ---";
		nvgText(args.vg, pos.x + 58 * 1, pos.y, text.c_str(), NULL);
		text = "min ";
		text += isNear(stats->vmin, 0.f, 100.f) ? string::f("% 6.2f", stats->vmin) : "  ---";
		nvgText(args.vg, pos.x + 58 * 2, pos.y, text.c_str(), NULL);
	}

	void draw(const DrawArgs &args) override {
		if (!module)
			return;

		float gainX = std::pow(2.f, std::round(module->params[Scope::X_SCALE_PARAM].getValue())) / 10.f;
		float gainY = std::pow(2.f, std::round(module->params[Scope::Y_SCALE_PARAM].getValue())) / 10.f;
		float offsetX = module->params[Scope::X_POS_PARAM].getValue();
		float offsetY = module->params[Scope::Y_POS_PARAM].getValue();

		// Draw waveforms
		if (module->lissajous) {
			// X x Y
			int lissajousChannels = std::max(module->channelsX, module->channelsY);
			for (int c = 0; c < lissajousChannels; c++) {
				drawWaveform(args,
										 module->bufferX[c],
										 offsetX,
										 gainX,
										 module->bufferY[c],
										 offsetY,
										 gainY,
										 0,
										 0,
										 nvgHSLA(module->hue, 0.5f, 0.5f, 200));

				//draw Kaleidoscope rotations;
				auto unitRotation = (float) (2.0 * M_PI) / (float) module->kaleidoscope.count;
				auto reflectionCount = !module->kaleidoscope.use ? 0 : module->kaleidoscope.count;
				for (auto i = 0; i < reflectionCount; ++i) {
					drawWaveform(args,
											 module->bufferX[c],
											 offsetX,
											 gainX,
											 module->bufferY[c],
											 offsetY,
											 gainY,
											 module->kaleidoscope.radius,
											 (i * unitRotation),
											 nvgHSLA(module->hue, 0.5f, 0.5f, 200));
				}
			}
		} else {
			// Y
			for (int c = 0; c < module->channelsY; c++) {
				drawWaveform(args,
										 NULL,
										 0,
										 0,
										 module->bufferY[c],
										 offsetY,
										 gainY,
										 0,
										 0,
										 nvgRGBA(0xe1, 0x02, 0x78, 0xc0));
			}

			// X
			for (int c = 0; c < module->channelsX; c++) {
				drawWaveform(args,
										 NULL,
										 0,
										 0,
										 module->bufferX[c],
										 offsetX,
										 gainX,
										 0,
										 0,
										 nvgHSLA(module->hue, 0.5f, 0.5f, 200));
			}

			float trigThreshold = module->params[Scope::TRIG_PARAM].getValue();
			trigThreshold = (trigThreshold + offsetX) * gainX;
			drawTrig(args, trigThreshold);
		}

		// Calculate and draw stats
		if (++statsFrame >= 4) {
			statsFrame = 0;
			statsX.calculate(module->bufferX[0], module->channelsX);
			statsY.calculate(module->bufferY[0], module->channelsY);
		}
		drawStats(args, Vec(25, 0), "X", &statsX);
		drawStats(args, Vec(25, box.size.y - 15), "Y", &statsY);
	}
};

struct PlotTypeMenuItem : MenuItem {
	Scope *module;
	bool lissajous = false;
	bool kaleidoscope = false;

	void onAction(const event::Action &e) override {
		module->lissajous = lissajous;
		module->kaleidoscope.use = kaleidoscope;
	}
};

struct LineTypeMenuItem : MenuItem {
	Scope *module;
	Scope::LineType lineType = Scope::LineType::NORMAL_LINE;

	void onAction(const event::Action &e) override {
		module->params[Scope::LINE_TYPE_PARAM].setValue(lineType);
	}
};

struct ExternalTriggerMenuItem : MenuItem {
	Scope *module;

	void onAction(const event::Action &e) override {
		module->external = !module->external;
	}
};

struct LineFadeMenuItem : MenuItem {
	Scope *module;

	void onAction(const event::Action &e) override {
		module->params[Scope::LINE_FADE_PARAM].setValue(!module->params[Scope::LINE_FADE_PARAM].getValue());
	}
};

struct TinyKnob : RoundKnob {
	TinyKnob() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/scopeTinyKnob.svg")));
	}
};

struct TinySnapKnob : RoundKnob {
	TinySnapKnob() {
		snap = true;
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/scopeTinyKnob.svg")));
	}
};

struct TinyPort : app::SvgPort {
	TinyPort() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/scopeTinyPort.svg")));
	}
};

// Components

struct ScopeWidget : ModuleWidget {
	ResizeTab rt;
	ScopeDisplay *display;

	ScopeWidget(Scope *module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * 17, RACK_GRID_HEIGHT);
		{
			panel = new ScopePanel(nvgRGB(20, 30, 33));
			panel->box.size = box.size;
			addChild(panel);
		}
		{
			auto leftBorder = 0;
			display = new ScopeDisplay();
			display->module = module;
			display->box.pos = Vec(leftBorder, 0);
			display->box.size = Vec(box.size.x - leftBorder, box.size.y);
			addChild(display);
		}
		//add resize tab to ui
		rt.box.pos.x = box.size.x - RACK_GRID_WIDTH;
		rt.box.pos.y = 0.0f;
		addChild(&rt);

		addInput(createInputCentered<TinyPort>(mm2px(Vec(5, 5)), module, Scope::X_INPUT));
		addParam(createParamCentered<TinyKnob>(mm2px(Vec(5, 12.5)), module, Scope::X_SCALE_PARAM));
		addParam(createParamCentered<TinyKnob>(mm2px(Vec(5, 20)), module, Scope::X_POS_PARAM));
		addInput(createInputCentered<TinyPort>(mm2px(Vec(5, 27.5)), module, Scope::Y_INPUT));
		addParam(createParamCentered<TinyKnob>(mm2px(Vec(5, 35)), module, Scope::Y_SCALE_PARAM));
		addParam(createParamCentered<TinyKnob>(mm2px(Vec(5, 42.5)), module, Scope::Y_POS_PARAM));
		addParam(createParamCentered<TinyKnob>(mm2px(Vec(5, 50)), module, Scope::TIME_PARAM));
		addInput(createInputCentered<TinyPort>(mm2px(Vec(5, 57.5)), module, Scope::TRIG_INPUT));
		addParam(createParamCentered<TinyKnob>(mm2px(Vec(5, 65)), module, Scope::TRIG_PARAM));
		addInput(createInputCentered<TinyPort>(mm2px(Vec(5, 72.5)), module, Scope::COLOR_INPUT));
		addParam(createParamCentered<TinyKnob>(mm2px(Vec(5, 80)), module, Scope::LINE_COLOR_PARAM));
		addInput(createInputCentered<TinyPort>(mm2px(Vec(5, 87.5)), module, Scope::LINE_WIDTH_INPUT));
		addParam((createParamCentered<TinyKnob>(mm2px(Vec(5, 95)), module, Scope::LINE_WIDTH_PARAM)));
		addInput(createInputCentered<TinyPort>(mm2px(Vec(5, 102.5)), module, Scope::KALEIDOSCOPE_COUNT_INPUT));
		addParam(createParamCentered<TinySnapKnob>(mm2px(Vec(5, 110)), module, Scope::KALEIDOSCOPE_COUNT_PARAM));
		addInput(createInputCentered<TinyPort>(mm2px(Vec(5, 117.5)), module, Scope::KALEIDOSCOPE_RADIUS_INPUT));
		addParam(createParamCentered<TinyKnob>(mm2px(Vec(5, 125)), module, Scope::KALEIDOSCOPE_RADIUS_PARAM));
	}

	virtual ~ScopeWidget() {
		removeChild(&rt);
	}

	void step() override {
		if (box.size.x != panel->box.size.x) { // ui resized
			if (module)
				((Scope *) (module))->widgetWidth.store(box.size.x);
		}

		if (module)
			box.size.x = ((Scope *) (module))->widgetWidth.load();

		panel->setSize(box.size);

		display->box.size = Vec(box.size.x, box.size.y);
		rt.box.pos.x = box.size.x - rt.box.size.x;
		rt.box.pos.y = 0;
		ModuleWidget::step();
	}

	void appendContextMenu(Menu *menu) override {
		auto *module = dynamic_cast<Scope *> (this->module);

		menu->addChild(new MenuEntry);

		MenuLabel *plotTypeLabel = new MenuLabel();
		plotTypeLabel->text = "Plot Type";
		menu->addChild(plotTypeLabel);

		PlotTypeMenuItem *normalPlotType = new PlotTypeMenuItem();
		normalPlotType->kaleidoscope = false;
		normalPlotType->lissajous = false;
		normalPlotType->text = "Normal";
		normalPlotType->rightText = CHECKMARK(module->lissajous == false && module->kaleidoscope.use == false);
		normalPlotType->module = module;
		menu->addChild(normalPlotType);

		PlotTypeMenuItem *lissajousPlotType = new PlotTypeMenuItem();
		lissajousPlotType->kaleidoscope = false;
		lissajousPlotType->lissajous = true;
		lissajousPlotType->text = "Lissajous";
		lissajousPlotType->rightText = CHECKMARK(module->lissajous == true && module->kaleidoscope.use == false);
		lissajousPlotType->module = module;
		menu->addChild(lissajousPlotType);

		PlotTypeMenuItem *kaleidoscope = new PlotTypeMenuItem();
		kaleidoscope->kaleidoscope = true;
		kaleidoscope->lissajous = true;
		kaleidoscope->text = "Kaleidoscope";
		kaleidoscope->rightText = CHECKMARK(module->lissajous == true && module->kaleidoscope.use == true);
		kaleidoscope->module = module;
		menu->addChild(kaleidoscope);

		menu->addChild(new MenuEntry);

		MenuLabel *triggerTypeLabel = new MenuLabel();
		triggerTypeLabel->text = "Trigger";
		menu->addChild(triggerTypeLabel);

		ExternalTriggerMenuItem *extTrig = new ExternalTriggerMenuItem();
		extTrig->text = "External";
		extTrig->rightText = CHECKMARK(module->external);
		extTrig->module = module;
		menu->addChild(extTrig);

		menu->addChild(new MenuEntry);

		MenuLabel *lineTypeLabel = new MenuLabel();
		lineTypeLabel->text = "LineType";
		menu->addChild(lineTypeLabel);

		LineTypeMenuItem *normalLineType = new LineTypeMenuItem();
		normalLineType->text = "Normal";
		normalLineType->lineType = Scope::LineType::NORMAL_LINE;
		normalLineType->rightText = CHECKMARK(module->lineType == Scope::LineType::NORMAL_LINE);
		normalLineType->module = module;
		menu->addChild(normalLineType);

		LineTypeMenuItem *vectorLineType = new LineTypeMenuItem();
		vectorLineType->text = "Vector";
		vectorLineType->lineType = Scope::LineType::VECTOR_LINE;
		vectorLineType->rightText = CHECKMARK(module->lineType == Scope::LineType::VECTOR_LINE);
		vectorLineType->module = module;
		menu->addChild(vectorLineType);

		LineTypeMenuItem *experimentalLineType = new LineTypeMenuItem();
		experimentalLineType->text = "Experimental";
		experimentalLineType->lineType = Scope::LineType::EXPERIMENTAL_LINE;
		experimentalLineType->rightText = CHECKMARK(module->lineType == Scope::LineType::EXPERIMENTAL_LINE);
		experimentalLineType->module = module;
		menu->addChild(experimentalLineType);

		LineFadeMenuItem *fade = new LineFadeMenuItem();
		fade->text = "Fade";
		fade->rightText = CHECKMARK(module->params[Scope::LINE_FADE_PARAM].getValue());
		fade->module = module;
		menu->addChild(fade);
	}
};

Model *modelLightScope = createModel<Scope, ScopeWidget>("LightScope");
