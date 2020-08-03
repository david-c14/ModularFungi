//Copyright (c) 2018 Andrew Belt and licensed under BSD-3-Clause by Andrew Belt

//Permission is hereby granted, free of charge, to any person obtaining a copy of this
// software and associated documentation files (the "Software"), to deal in the Software
// without restriction, including without limitation the rights to use, copy, modify,
// merge, publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so, subject to the
// following conditions:

//The above copyright notice and this permission notice shall be included in all copies
// or substantial portions of the Software.

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


//Modified by Dave French 2020
//Added use with lights off module, kaleidoscope plots, resizable, line types, line widths & fading lines
//reworked UI, moved options to context menu.

//Popout window code by Richie Hindle

#include <cstring>
#include <memory>
#include <atomic>
#include "ModularFungi.hpp"

// Get the GLFW API.
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>


///512 in original scope, 4096 with variable bufferSize
// This improves the drawing resolution, but reduces performance
// The size of the buffer use is chooses by the performace options
// in the context menu
static const auto MAX_BUFFER_SIZE = 4096;

struct Scope : Module {
	enum ParamIds {
		X_SCALE_PARAM,
		X_POS_PARAM,
		Y_SCALE_PARAM,
		Y_POS_PARAM,
		TIME_PARAM,
		LISSAJOUS_PARAM,  //unused, kept for 1.1.compatibility
		TRIG_PARAM,
		EXTERNAL_PARAM,
		KALEIDOSCOPE_USE_PARAM, //unused, kept for 1.1.compatibility
		KALEIDOSCOPE_COUNT_PARAM,
		KALEIDOSCOPE_RADIUS_PARAM,
		KALEIDOSCOPE_COLOR_SPREAD_PARAM,
		LINE_WIDTH_PARAM,
		LINE_FADE_PARAM,
		LINE_HUE_PARAM,
		LINE_TYPE_PARAM,
		SHOW_STATS_PARAM,
		SHOW_LABELS_PARAM,
		PLOT_TYPE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		X_INPUT,
		Y_INPUT,
		TRIG_INPUT,
		HUE_INPUT,
		LINE_WIDTH_INPUT,
		KALEIDOSCOPE_COUNT_INPUT,
		KALEIDOSCOPE_RADIUS_INPUT,
		KALEIDOSCOPE_COLOR_SPREAD_INPUT,
		X_SCALE_INPUT,
		Y_SCALE_INPUT,
		X_POS_INPUT,
		Y_POS_INPUT,
		TIME_INPUT,
		TRIG_LEVEL_INPUT,
		LINE_TYPE_INPUT,
		PLOT_TYPE_INPUT,
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
		VECTOR_LINE,
		EXPERIMENTAL_LINE,
		NUM_LINES
	};

	enum PlotType {
		NORMAL,
		LISSAJOUS,
		KALEIDOSCOPE,
		NUM_PLOT_TYPES
	};

	float bufferX[PORT_MAX_CHANNELS][MAX_BUFFER_SIZE] = {};
	float bufferY[PORT_MAX_CHANNELS][MAX_BUFFER_SIZE] = {};
	int channelsX = 0;
	int channelsY = 0;
	int bufferIndex = 0;
	int frameIndex = 0;
	int bufferSize = 512;

	//parameters for kaleidoscope
	struct Kaleidoscope {
		int count = 3;
		float radius = 20.0f;
	} kaleidoscope;

	dsp::SchmittTrigger triggers[16];

	// used to calculate parameter + cv values
	float hue = 0.5f;
	float lineWidth = 1.5f;
	float fade = 1.0f;
	std::atomic<float> widgetWidth;

	Scope() {
		widgetWidth.store(RACK_GRID_WIDTH * 20);

		const auto timeBase = (float) MAX_BUFFER_SIZE / 6;
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(X_SCALE_PARAM, -2.f, 8.f, 0.f, "X scale", " ", 1 / 2.f, 5);
		configParam(X_POS_PARAM, -10.f, 10.f, 0.f, "X position", " ");
		configParam(Y_SCALE_PARAM, -2.f, 8.f, 0.f, "Y scale", " ", 1 / 2.f, 5);
		configParam(Y_POS_PARAM, -10.f, 10.f, 0.f, "Y position", " ");
		configParam(TIME_PARAM, 6.f, 16.f, 14.f, "Time", " ", 1 / 2.f, 1000 * timeBase);
		configParam(LISSAJOUS_PARAM, 0.f, 1.f, 0.f, "X & Y / Lissajous");
		configParam(TRIG_PARAM, -10.f, 10.f, 0.f, "Trigger position", " V");
		configParam(EXTERNAL_PARAM, 0.f, 1.f, 0.f, "Internal / External Trigger");

		configParam(KALEIDOSCOPE_USE_PARAM, 0.0f, 1.0f, 0.0f, "Kaleidoscope");
		configParam(KALEIDOSCOPE_COUNT_PARAM, 3.0f, 12.0f, 3.0f, "Mirrors");
		configParam(KALEIDOSCOPE_RADIUS_PARAM, 1.0f, 200.0f, 1.0f, "Radius");

		configParam(LINE_TYPE_PARAM, 0.0f, NUM_LINES - 1, Scope::LineType::NORMAL_LINE, "Line Type");
		configParam(LINE_WIDTH_PARAM, 0.1f, 10.0f, 1.5f, "Line Width");
		configParam(LINE_HUE_PARAM, 0.0f, 1.0f, 1.0f, "Color");
		configParam(LINE_FADE_PARAM, 0.0f, 1.0f, 1.0f, "Fade");
		configParam(SHOW_STATS_PARAM, 0.0f, 1.0f, 0.0f, "Show Stats");
		configParam(KALEIDOSCOPE_COLOR_SPREAD_PARAM, 0.0f, 1.0f, 0.0f, "Kaleidoscope Color Spread");
		configParam(SHOW_LABELS_PARAM, 0.0f, 1.0f, 0.0f, "Show Labels");
		configParam(PLOT_TYPE_PARAM, 0.0f, NUM_PLOT_TYPES - 1, Scope::PlotType::NORMAL, "Plot Type");
	}

	void onReset() override {
		params[LISSAJOUS_PARAM].setValue(false);
		params[EXTERNAL_PARAM].setValue(false);
		params[KALEIDOSCOPE_USE_PARAM].setValue(false);
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

		hue = params[LINE_HUE_PARAM].getValue() + inputs[HUE_INPUT].getVoltage() / 10.0f;
		lineWidth = params[LINE_WIDTH_PARAM].getValue() + inputs[LINE_WIDTH_INPUT].getVoltage();
		fade = params[LINE_FADE_PARAM].getValue();

		//PLOT_TYPE_PARAM added after version 1.1.1
		//KALEIDOSCOPE_USE_PARAM and LISSAJOUS_PARAM are updated for compatibility
		auto pType = (int) (params[PLOT_TYPE_PARAM].getValue() + inputs[PLOT_TYPE_INPUT].getVoltage() / 3.0f);
		pType = clamp(pType, 0, NUM_PLOT_TYPES - 1);
		switch ((PlotType) pType) {
			case PlotType::NORMAL:
				params[KALEIDOSCOPE_USE_PARAM].setValue(false);
				params[LISSAJOUS_PARAM].setValue(false);
				break;
			case PlotType::KALEIDOSCOPE:
				params[KALEIDOSCOPE_USE_PARAM].setValue(true);
				params[LISSAJOUS_PARAM].setValue(true);
				break;
			case PlotType::LISSAJOUS:
				params[KALEIDOSCOPE_USE_PARAM].setValue(false);
				params[LISSAJOUS_PARAM].setValue(true);
				break;
			default:
				params[KALEIDOSCOPE_USE_PARAM].setValue(false);
				params[LISSAJOUS_PARAM].setValue(false);
				break;
		}

		// Compute time
		//updated to use cv
		auto deltaTime = std::pow(2.f,
								  -clamp(params[TIME_PARAM].getValue() + abs(inputs[TIME_INPUT].getVoltage()), 6.0f,
										 16.0f));
		auto frameCount = (int) std::ceil(deltaTime * args.sampleRate);

		// Set channels
		auto channelsX = inputs[X_INPUT].getChannels();
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
		if (bufferIndex < bufferSize) {
			if (++frameIndex > frameCount) {
				frameIndex = 0;
				for (auto c = 0; c < channelsX; c++) {
					bufferX[c][bufferIndex] = inputs[X_INPUT].getVoltage(c);
				}
				for (auto c = 0; c < channelsY; c++) {
					bufferY[c][bufferIndex] = inputs[Y_INPUT].getVoltage(c);
				}
				bufferIndex++;
			}
		}

		// Don't wait for trigger if still filling buffer
		if (bufferIndex < bufferSize) {
			return;
		}

		// Trigger immediately if external but nothing plugged in, or in Lissajous mode
		if ((bool) params[LISSAJOUS_PARAM].getValue()
			|| ((bool) params[EXTERNAL_PARAM].getValue() && !inputs[TRIG_INPUT].isConnected())) {
			trigger();
			return;
		}

		frameIndex++;

		// Reset if triggered
		auto trigThreshold = params[TRIG_PARAM].getValue();
		trigThreshold += inputs[Scope::TRIG_LEVEL_INPUT].getVoltage();
		trigThreshold = clamp(trigThreshold, -10.0f, 10.0f);
		Input &trigInput = (bool) params[EXTERNAL_PARAM].getValue() ? inputs[TRIG_INPUT] : inputs[X_INPUT];

		// This may be 0
		auto trigChannels = trigInput.getChannels();
		for (auto c = 0; c < trigChannels; c++) {
			auto trigVoltage = trigInput.getVoltage(c);
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
		for (auto &trigger : triggers) {
			trigger.reset();
		}
		bufferIndex = 0;
		frameIndex = 0;
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "WidgetWidth", json_real(widgetWidth.load()));
		json_object_set_new(rootJ, "bufferSize", json_integer(bufferSize));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t *ww = json_object_get(rootJ, "WidgetWidth");
		if (ww)
			widgetWidth.store((float) json_real_value(ww));

		json_t *bs = json_object_get(rootJ, "bufferSize");
		if (bs)
			bufferSize = json_integer_value(bs);
	}
};

// USER INTERFACE  ****************

/// interface to be implemented by module widget for popout window
struct IPopupWindowOwner
{
	virtual void IPopupWindowOwner_showWindow() = 0;
	virtual void IPopupWindowOwner_hideWindow() = 0;
};

/// Placed on right of module, allow resizing of parent widget via drag
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

		//try to resize widget, or return to current size
		modWidget->box = newRect;
		if (!APP->scene->rack->requestModulePos(modWidget, newRect.pos))
			modWidget->box = oldRect;
	}
};

// No svg background is used
// ScopePanel is used to draw background and border
// allowing for resizeable widget
struct ScopePanel : Widget {
	Widget *panelBorder = nullptr;
	NVGcolor backGroundColor;

	ScopePanel(NVGcolor color) {
		panelBorder = new PanelBorder;
		backGroundColor = color;
		addChild(panelBorder);
	}

	void step() override {
		//
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
	Vec lastCoordinate;
	bool externalWindow = false;

	struct Stats {
		float vpp = 0.f;
		float vMin = 0.f;
		float vMax = 0.f;

		void calculate(float *buffer, int channels) {
			vMax = -INFINITY;
			vMin = INFINITY;
			for (auto i = 0; i < MAX_BUFFER_SIZE * channels; i++) {
				auto v = buffer[i];
				vMax = std::fmax(vMax, v);
				vMin = std::fmin(vMin, v);
			}
			vpp = vMax - vMin;
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
					  NVGcolor beam = {1.0f, 1.0f, 1.0f, 1.0f},
					  Rect bounds = {0,0,1,1}) {
		assert(bufferY);
		nvgSave(args.vg);

		//beam fading using a varying alpha
		auto maxAlpha = 0.99f;
		auto lightInc = maxAlpha / (float) module->bufferSize;
		auto currentAlpha = maxAlpha;

		auto currentLineWidth = module->lineWidth;
		auto widthInc = module->lineWidth / (float) module->bufferSize;;


		auto b = Rect(Vec(0, 15), bounds.size.minus(Vec(0, 15 * 2)));
		nvgBeginPath(args.vg);
		nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
		nvgTranslate(args.vg, kRadius * simd::cos(kRotation) + bounds.size.x / 2.0f,
					 kRadius * simd::sin(kRotation) - (bounds.size.y - 30) / 2.0f);
		if (!(bool) module->params[Scope::LISSAJOUS_PARAM].getValue())
			nvgTranslate(args.vg, -bounds.size.x / 2.0f, 0);

		nvgLineCap(args.vg, NVG_BUTT);
		nvgMiterLimit(args.vg, 2.f);
		nvgStrokeWidth(args.vg, module->lineWidth);
		nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);

		auto cos2R = simd::cos(2.0f * kRotation);
		auto sin2R = simd::sin(2.0f * kRotation);

		// when drawing the buffer, if the line is to fade, start drawing at 2 samples prior
		// bufferIndex, with full alpha.
		// when the line is not fading, draw the buffer from end to start to remove flicker.
		auto startIndex = (bool) module->fade ? module->bufferIndex - 3 : module->bufferSize - 2;
		startIndex = clamp(startIndex, 0, module->bufferSize - 1);
		auto endIndex = (bool) module->fade ? module->bufferIndex - 2 : 0;
		endIndex = clamp(endIndex, 1, module->bufferSize - 1);


		for (auto i = startIndex; i != endIndex; i--) {
			if (i < 0)
				i = module->bufferSize - 1; // loop buffer due to starting at various locations

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
				v.x = (float) i / (module->bufferSize - 1);
			}
			v.y = (bufferY[i] + offsetY) * gainY / 2.0f;

			//rotate 2 * kRotate

			auto xNew = v.x * cos2R + v.y * sin2R;
			v.y = -v.x * sin2R + v.y * cos2R;
			v.x = xNew;

			Vec p;
			// resize x & y by height, to keep plots in correct ratio if Lissajous
			if ((bool) module->params[Scope::LISSAJOUS_PARAM].getValue())
				p.x = rescale(v.x, 0.f, 1.f, b.pos.x, b.pos.y + b.size.y);
			else
				p.x = rescale(v.x, 0.f, 1.f, b.pos.x, b.pos.x + b.size.x);

			p.y = rescale(v.y, 0.f, 1.f, b.pos.y + b.size.y, b.pos.y);
			if (i == module->bufferSize - 1) {
				nvgMoveTo(args.vg, p.x, p.y);
				lastCoordinate = p;
			} else {
				auto vectorScale = 0.998f;
				auto experimentalScale = 0.9f;
				auto lType = (int) (module->params[Scope::LINE_TYPE_PARAM].getValue()
									+ module->inputs[Scope::LINE_TYPE_INPUT].getVoltage());
				lType = clamp(lType, 0, (int) Scope::LineType::NUM_LINES - 1);
				switch ((Scope::LineType) lType) {
					case Scope::LineType::NORMAL_LINE: {
						//morph between normal and vector lines
						auto normVecCoeff = (module->params[Scope::LINE_TYPE_PARAM].getValue()
											 + module->inputs[Scope::LINE_TYPE_INPUT].getVoltage())
											- (int) Scope::LineType::NORMAL_LINE;
						Vec intermediate;
						intermediate.x = normVecCoeff * (p.x - lastCoordinate.x) + lastCoordinate.x;
						intermediate.y = normVecCoeff * (p.y - lastCoordinate.y) + lastCoordinate.y;
						if (i != startIndex)
							nvgMoveTo(args.vg, intermediate.x, intermediate.y);

						nvgLineTo(args.vg, p.x, p.y);
						break;
					}
					case Scope::LineType::VECTOR_LINE: {
						//morph between vector and experimental line types
						auto vecExprCoeff = (module->params[Scope::LINE_TYPE_PARAM].getValue()
											 + module->inputs[Scope::LINE_TYPE_INPUT].getVoltage())
											- (int) Scope::LineType::VECTOR_LINE;
						auto scale = vecExprCoeff * (experimentalScale - vectorScale) + vectorScale;

						nvgMoveTo(args.vg, scale * p.x, scale * p.y);
						nvgLineTo(args.vg, p.x, p.y);
						break;
					}
					case Scope::LineType::EXPERIMENTAL_LINE:
						nvgMoveTo(args.vg, experimentalScale * p.x, experimentalScale * p.y);
						nvgLineTo(args.vg, p.x, p.y);
						break;
					case Scope::NUM_LINES:
						assert(false);
						break;
					default:
						assert(false);
				}
				lastCoordinate = p;
			}
			nvgStroke(args.vg);
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, p.x, p.y);
			lastCoordinate = p;
		}
		nvgResetTransform(args.vg);
		nvgResetScissor(args.vg);
		nvgRestore(args.vg);
	}

	void drawTrig(const DrawArgs &args, float value, Rect bounds) {
		auto b = Rect(Vec(0, 15), bounds.size.minus(Vec(0, 15 * 2)));
		nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);

		value = value / 2.f + 0.5f;
		auto p = Vec(bounds.size.x, b.pos.y + b.size.y * (1.f - value));

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
		text += isNear(stats->vMax, 0.f, 100.f) ? string::f("% 6.2f", stats->vMax) : "  ---";
		nvgText(args.vg, pos.x + 58 * 1, pos.y, text.c_str(), NULL);
		text = "min ";
		text += isNear(stats->vMin, 0.f, 100.f) ? string::f("% 6.2f", stats->vMin) : "  ---";
		nvgText(args.vg, pos.x + 58 * 2, pos.y, text.c_str(), NULL);
	}

	void drawLabels(const DrawArgs &args) {
		std::vector<std::string> labels = {"X Input", "X Scale", "X Position", "Y Input", "Y Scale", "Y Position",
										   "Time", "Trigger Input", "Trigger Position", "Color", "Line Width",
										   "Kaleidoscope Images", "Kaleidoscope Radius", "Color Spread", "Line Type",
										   "Plot Type"};
		nvgFontSize(args.vg, 13);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, -2);

		nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0x80));
		auto y = 6.5f;
		for (const auto &l : labels) {
			nvgText(args.vg, mm2px(10.0f), mm2px(y), l.c_str(), NULL);
			y += 7.5;
		}
	}

	void draw(const DrawArgs &args) override {
		if (!module)
			return;

		// only display woweform in widget if the external window
		// is not open. The external window is drawn from ScopeWidget::step
		// where additioanl comments are found
		if (!externalWindow)
			preDrawWaveforms(args, box);

		// Calculate and draw stats
		if ((bool) module->params[Scope::SHOW_STATS_PARAM].getValue()) {
			if (++statsFrame >= 4) {
				statsFrame = 0;
				statsX.calculate(module->bufferX[0], module->channelsX);
				statsY.calculate(module->bufferY[0], module->channelsY);
			}
			drawStats(args, Vec(25, 0), "X", &statsX);
			drawStats(args, Vec(25, box.size.y - 15), "Y", &statsY);
		}

		if ((bool) module->params[Scope::SHOW_LABELS_PARAM].getValue()) {
			drawLabels(args);
		}

		LightWidget::draw(args);
	}

	void preDrawWaveforms(const DrawArgs &args, Rect bounds) {
		auto gainX = std::pow(2.f, module->params[Scope::X_SCALE_PARAM].getValue()) / 10.0f;
		gainX += module->inputs[Scope::X_SCALE_INPUT].getVoltage() / 10.0f;
		auto gainY = std::pow(2.f, module->params[Scope::Y_SCALE_PARAM].getValue()) / 10.0f;
		gainY += module->inputs[Scope::Y_SCALE_INPUT].getVoltage() / 10.0f;
		auto offsetX = module->params[Scope::X_POS_PARAM].getValue();
		offsetX += module->inputs[Scope::X_POS_INPUT].getVoltage();
		auto offsetY = module->params[Scope::Y_POS_PARAM].getValue();
		offsetY += module->inputs[Scope::Y_POS_INPUT].getVoltage();

		// Draw waveforms
		if ((bool) module->params[Scope::LISSAJOUS_PARAM].getValue()) {
			// X x Y
			auto lissajousChannels = std::max(module->channelsX, module->channelsY);
			for (auto c = 0; c < lissajousChannels; c++) {
				drawWaveform(args,
							 module->bufferX[c],
							 offsetX,
							 gainX,
							 module->bufferY[c],
							 offsetY,
							 gainY,
							 0,
							 0,
							 nvgHSLA(module->hue, 0.5f, 0.5f, 200),
							 bounds);

				//draw Kaleidoscope rotations;
				auto unitRotation = (float) (2.0 * M_PI) / (float) module->kaleidoscope.count;
				auto reflectionCount = !(bool) module->params[Scope::KALEIDOSCOPE_USE_PARAM].getValue()
									   ? 0
									   : module->kaleidoscope.count;
				auto unitHueChange = (module->params[Scope::KALEIDOSCOPE_COLOR_SPREAD_PARAM].getValue()
									  + module->inputs[Scope::KALEIDOSCOPE_COLOR_SPREAD_INPUT].getVoltage() / 5.0)
									 / reflectionCount;

				for (auto i = 0; i < reflectionCount; ++i) {
					auto hueChange = (i + 1) * unitHueChange;
					auto reflectionHue = std::fmod(module->hue + hueChange, 1.0f);
					drawWaveform(args,
								 module->bufferX[c],
								 offsetX,
								 -gainX,
								 module->bufferY[c],
								 offsetY,
								 gainY,
								 module->kaleidoscope.radius,
								 (i * unitRotation),
								 nvgHSLA(reflectionHue, 0.5f, 0.5f, 200),
								 bounds);
				}
			}
		} else {  //draw normal
			// Y
			for (auto c = 0; c < module->channelsY; c++) {
				drawWaveform(args,
							 NULL,
							 0,
							 0,
							 module->bufferY[c],
							 offsetY,
							 gainY,
							 0,
							 0,
							 nvgRGBA(0xe1, 0x02, 0x78, 0xc0),
							 bounds);
			}

			// X
			for (auto c = 0; c < module->channelsX; c++) {
				drawWaveform(args,
							 NULL,
							 0,
							 0,
							 module->bufferX[c],
							 offsetX,
							 gainX,
							 0,
							 0,
							 nvgHSLA(module->hue, 0.5f, 0.5f, 200),
							 bounds);
			}

			auto trigThreshold = module->params[Scope::TRIG_PARAM].getValue();
			trigThreshold += module->inputs[Scope::TRIG_LEVEL_INPUT].getVoltage();
			trigThreshold = clamp(trigThreshold, -10.0f, 10.0f);
			trigThreshold = (trigThreshold + offsetX) * gainX;
			drawTrig(args, trigThreshold, bounds);
		}
	}
};

//Context menus

struct ShowWindowMenuItem : MenuItem {
	IPopupWindowOwner* windowOwner;
	void onAction(const event::Action& e) override {
		windowOwner->IPopupWindowOwner_showWindow();
	}
};

struct HideWindowMenuItem : MenuItem {
	IPopupWindowOwner* windowOwner;
	void onAction(const event::Action& e) override {
		windowOwner->IPopupWindowOwner_hideWindow();
	}
};

struct PlotTypeMenuItem : MenuItem {
	Scope *module;
	Scope::PlotType plotType = Scope::PlotType::NORMAL;

	void onAction(const event::Action &e) override {
		module->params[Scope::PLOT_TYPE_PARAM].setValue(plotType);
	}
};

struct LineTypeMenuItem : MenuItem {
	Scope *module;
	Scope::LineType lineType = Scope::LineType::NORMAL_LINE;

	void onAction(const event::Action &e) override {
		module->params[Scope::LINE_TYPE_PARAM].setValue(lineType);
	}
};

struct ResolutionMenuItem : MenuItem {
	Scope *module;
	int size = 512;

	void onAction(const event::Action &e) override {
		module->bufferSize = size;
	}
};

struct ExternalTriggerMenuItem : MenuItem {
	Scope *module;

	void onAction(const event::Action &e) override {
		module->params[Scope::EXTERNAL_PARAM].setValue
				(!(bool) module->params[Scope::EXTERNAL_PARAM].getValue());
	}
};

struct LineFadeMenuItem : MenuItem {
	Scope *module;

	void onAction(const event::Action &e) override {
		module->params[Scope::LINE_FADE_PARAM].setValue
				(!(bool) module->params[Scope::LINE_FADE_PARAM].getValue());
	}
};

struct ShowStatsMenuItem : MenuItem {
	Scope *module;

	void onAction(const event::Action &e) override {
		module->params[Scope::SHOW_STATS_PARAM].setValue
				(!(bool) module->params[Scope::SHOW_STATS_PARAM].getValue());
	}
};

struct ShowLabelsMenuItem : MenuItem {
	Scope *module;

	void onAction(const event::Action &e) override {
		module->params[Scope::SHOW_LABELS_PARAM].setValue
				(!(bool) module->params[Scope::SHOW_LABELS_PARAM].getValue());
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

struct Port2mm : app::SvgPort {
	Port2mm() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/scope2mmPort.svg")));
	}
};

// Components

struct ScopeWidget : ModuleWidget, IPopupWindowOwner {
	ResizeTab rt;
	ScopeDisplay *display;
	GLFWwindow* _window = nullptr;  // Handle to the popup window.
	NVGcontext* _vg = nullptr;		// For painting into the popup window.
	std::shared_ptr<Font> _font;	//

	ScopeWidget(Scope *module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * 17, RACK_GRID_HEIGHT);
		{
			panel = new ScopePanel(nvgRGB(5, 5, 30));
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
		addInput(createInputCentered<Port2mm>(mm2px(Vec(5, 12.5)), module, Scope::X_SCALE_INPUT));
		addParam(createParamCentered<TinyKnob>(mm2px(Vec(5, 20)), module, Scope::X_POS_PARAM));
		addInput(createInputCentered<Port2mm>(mm2px(Vec(5, 20)), module, Scope::X_POS_INPUT));
		addInput(createInputCentered<TinyPort>(mm2px(Vec(5, 27.5)), module, Scope::Y_INPUT));
		addParam(createParamCentered<TinyKnob>(mm2px(Vec(5, 35)), module, Scope::Y_SCALE_PARAM));
		addInput(createInputCentered<Port2mm>(mm2px(Vec(5, 35)), module, Scope::Y_SCALE_INPUT));
		addParam(createParamCentered<TinyKnob>(mm2px(Vec(5, 42.5)), module, Scope::Y_POS_PARAM));
		addInput(createInputCentered<Port2mm>(mm2px(Vec(5, 42.5)), module, Scope::Y_POS_INPUT));
		addParam(createParamCentered<TinyKnob>(mm2px(Vec(5, 50)), module, Scope::TIME_PARAM));
		addInput(createInputCentered<Port2mm>(mm2px(Vec(5, 50)), module, Scope::TIME_INPUT));
		addInput(createInputCentered<TinyPort>(mm2px(Vec(5, 57.5)), module, Scope::TRIG_INPUT));
		addParam(createParamCentered<TinyKnob>(mm2px(Vec(5, 65)), module, Scope::TRIG_PARAM));
		addInput(createInputCentered<Port2mm>(mm2px(Vec(5, 65)), module, Scope::TRIG_LEVEL_INPUT));
		addParam(createParamCentered<TinyKnob>(mm2px(Vec(5, 72.5)), module, Scope::LINE_HUE_PARAM));
		addInput(createInputCentered<Port2mm>(mm2px(Vec(5, 72.5)), module, Scope::HUE_INPUT));
		addParam((createParamCentered<TinyKnob>(mm2px(Vec(5, 80.0)), module, Scope::LINE_WIDTH_PARAM)));
		addInput(createInputCentered<Port2mm>(mm2px(Vec(5, 80.0)), module, Scope::LINE_WIDTH_INPUT));
		addParam(createParamCentered<TinySnapKnob>(mm2px(Vec(5, 87.5)), module, Scope::KALEIDOSCOPE_COUNT_PARAM));
		addInput(createInputCentered<Port2mm>(mm2px(Vec(5, 87.5)), module, Scope::KALEIDOSCOPE_COUNT_INPUT));
		addParam(createParamCentered<TinyKnob>(mm2px(Vec(5, 95.0)), module, Scope::KALEIDOSCOPE_RADIUS_PARAM));
		addInput(createInputCentered<Port2mm>(mm2px(Vec(5, 95.0)), module, Scope::KALEIDOSCOPE_RADIUS_INPUT));
		addParam(createParamCentered<TinyKnob>(mm2px(Vec(5, 102.5)), module, Scope::KALEIDOSCOPE_COLOR_SPREAD_PARAM));
		addInput(createInputCentered<Port2mm>(mm2px(Vec(5, 102.5)), module, Scope::KALEIDOSCOPE_COLOR_SPREAD_INPUT));
		addParam(createParamCentered<TinyKnob>(mm2px(Vec(5, 110.0)), module, Scope::LINE_TYPE_PARAM));
		addInput(createInputCentered<Port2mm>(mm2px(Vec(5, 110.0)), module, Scope::LINE_TYPE_INPUT));
		addParam(createParamCentered<TinySnapKnob>(mm2px(Vec(5, 117.5)), module, Scope::PLOT_TYPE_PARAM));
		addInput(createInputCentered<Port2mm>(mm2px(Vec(5, 117.5)), module, Scope::PLOT_TYPE_INPUT));
	}

	~ScopeWidget() override {
		removeChild(&rt);
		// Hide the pop out window if we have one
		IPopupWindowOwner_hideWindow();
	}


	void step() override {
		//pop-out window is handled here, I would have preferred ScopeDisplay::draw()
		//but this only updates the external window if the ModuleWidget is displayed
		//zooming and scrolling in the main window can stop rendering of the external
		//window
		if (_window) {
			display->externalWindow = true;
			// Paint the popup window.
			glfwMakeContextCurrent(_window);

			// Get the size of the popup window, both the window and the framebuffer.
			int winWidth, winHeight;
			int fbWidth, fbHeight;
			float pxRatio;
			glfwGetWindowSize(_window, &winWidth, &winHeight);
			glfwGetFramebufferSize(_window, &fbWidth, &fbHeight);
			pxRatio = (float)fbWidth / (float)winWidth;

			// Start painting.
			glViewport(0, 0, fbWidth, fbHeight);
			glClearColor(0, 0, 0, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			nvgBeginFrame(_vg, (float)winWidth, (float)winHeight, pxRatio);

			DrawArgs context;
			context.vg = _vg;
			display->preDrawWaveforms(context, Rect(0, 0, fbWidth, fbHeight));

			// Finished painting.
			nvgEndFrame(_vg);
			glfwSwapBuffers(_window);
			glfwMakeContextCurrent(APP->window->win);

			// If the user has clicked the window's Close button, close it.
			if (glfwWindowShouldClose(_window)) {
				IPopupWindowOwner_hideWindow();
			}
		}
		else
			display->externalWindow=false;

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

	void IPopupWindowOwner_showWindow() override {
		if (_window == nullptr) {
			// Tell GLFW the properties of the window we want to create.
			glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
			glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
			glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

			// Create the window.
			_window = glfwCreateWindow(400, 300, "Opsylloscope", NULL, NULL);

			// Don't wait for vsync when rendering 'cos it slows down the Rack UI thread.
			glfwMakeContextCurrent(_window);
			glfwSwapInterval(0);

			// If you want your window to stay on top of other windows.
//			glfwSetWindowAttrib(_window, GLFW_FLOATING, true);

			// Create a NanoVG context for painting the popup window.
//			_vg = nvgCreateGL2(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
			_vg = nvgCreateGL2(0);

			// Hand OpenGL back to Rack.
			glfwMakeContextCurrent(APP->window->win);
		}
	}

	void IPopupWindowOwner_hideWindow() override {
		if (_window != nullptr) {
			// Destroy the window and its NanoVG context.
			glfwMakeContextCurrent(_window);
			nvgDeleteGL2(_vg);
			glfwDestroyWindow(_window);
			glfwMakeContextCurrent(APP->window->win);
			_window = nullptr;
		}
	}

	void appendContextMenu(Menu *menu) override {
		auto *module = dynamic_cast<Scope *> (this->module);

		menu->addChild(new MenuEntry);

		if (_window == nullptr) {
			ShowWindowMenuItem* showWindowMenuItem = new ShowWindowMenuItem;
			showWindowMenuItem->text = "Display in pop-out window";
			showWindowMenuItem->windowOwner = this;
			menu->addChild(showWindowMenuItem);
		} else {
			HideWindowMenuItem* hideWindowMenuItem = new HideWindowMenuItem;
			hideWindowMenuItem->text = "Hide pop-out window";
			hideWindowMenuItem->windowOwner = this;
			menu->addChild(hideWindowMenuItem);
		}

		menu->addChild(new MenuEntry);

		auto *plotTypeLabel = new MenuLabel();
		plotTypeLabel->text = "Plot Type";
		menu->addChild(plotTypeLabel);

		auto *normalPlotType = new PlotTypeMenuItem();
		normalPlotType->plotType = Scope::PlotType::NORMAL;
		normalPlotType->text = "Normal";
		normalPlotType->rightText = CHECKMARK(
				module->params[Scope::PLOT_TYPE_PARAM].getValue() == Scope::PlotType::NORMAL);
		normalPlotType->module = module;
		menu->addChild(normalPlotType);

		auto *lissajousPlotType = new PlotTypeMenuItem();
		lissajousPlotType->plotType = Scope::PlotType::LISSAJOUS;
		lissajousPlotType->text = "Lissajous";
		lissajousPlotType->rightText = CHECKMARK(
				module->params[Scope::PLOT_TYPE_PARAM].getValue() == Scope::PlotType::LISSAJOUS);
		lissajousPlotType->module = module;
		menu->addChild(lissajousPlotType);

		auto *kaleidoscope = new PlotTypeMenuItem();
		kaleidoscope->plotType = Scope::PlotType::KALEIDOSCOPE;
		kaleidoscope->text = "Kaleidoscope";
		kaleidoscope->rightText = CHECKMARK(
				module->params[Scope::PLOT_TYPE_PARAM].getValue() == Scope::PlotType::KALEIDOSCOPE);
		kaleidoscope->module = module;
		menu->addChild(kaleidoscope);

		menu->addChild(new MenuEntry);

		auto *lineTypeLabel = new MenuLabel();
		lineTypeLabel->text = "LineType";
		menu->addChild(lineTypeLabel);

		auto *normalLineType = new LineTypeMenuItem();
		normalLineType->text = "Normal";
		normalLineType->lineType = Scope::LineType::NORMAL_LINE;
		normalLineType->rightText = CHECKMARK(module->params[Scope::LINE_TYPE_PARAM].getValue()
											  == Scope::LineType::NORMAL_LINE);
		normalLineType->module = module;
		menu->addChild(normalLineType);

		auto *vectorLineType = new LineTypeMenuItem();
		vectorLineType->text = "Vector";
		vectorLineType->lineType = Scope::LineType::VECTOR_LINE;
		vectorLineType->rightText = CHECKMARK(module->params[Scope::LINE_TYPE_PARAM].getValue()
											  == Scope::LineType::VECTOR_LINE);
		vectorLineType->module = module;
		menu->addChild(vectorLineType);

		auto *experimentalLineType = new LineTypeMenuItem();
		experimentalLineType->text = "Experimental";
		experimentalLineType->lineType = Scope::LineType::EXPERIMENTAL_LINE;
		experimentalLineType->rightText = CHECKMARK(module->params[Scope::LINE_TYPE_PARAM].getValue()
													== Scope::LineType::EXPERIMENTAL_LINE);
		experimentalLineType->module = module;
		menu->addChild(experimentalLineType);

		menu->addChild(new MenuEntry);

		auto *triggerTypeLabel = new MenuLabel();
		triggerTypeLabel->text = "Trigger";
		menu->addChild(triggerTypeLabel);

		auto *extTrig = new ExternalTriggerMenuItem();
		extTrig->text = "External";
		extTrig->rightText = CHECKMARK(module->params[Scope::EXTERNAL_PARAM].getValue());
		extTrig->module = module;
		menu->addChild(extTrig);

		menu->addChild(new MenuEntry);

		auto *fade = new LineFadeMenuItem();
		fade->text = "Fade";
		fade->rightText = CHECKMARK(module->params[Scope::LINE_FADE_PARAM].getValue());
		fade->module = module;
		menu->addChild(fade);

		auto *showStats = new ShowStatsMenuItem();
		showStats->text = "Show Stats";
		showStats->rightText = CHECKMARK(module->params[Scope::SHOW_STATS_PARAM].getValue());
		showStats->module = module;
		menu->addChild(showStats);

		auto *showLabels = new ShowLabelsMenuItem();
		showLabels->text = "Show Labels";
		showLabels->rightText = CHECKMARK(module->params[Scope::SHOW_LABELS_PARAM].getValue());
		showLabels->module = module;
		menu->addChild(showLabels);

		menu->addChild(new MenuEntry);

		auto *bufferSizeLabel = new MenuLabel();
		bufferSizeLabel->text = "Performance";
		menu->addChild(bufferSizeLabel);

		auto *resolution512 = new ResolutionMenuItem();
		resolution512->module = module;
		resolution512->size = 512;
		resolution512->text = "Eco";
		resolution512->rightText = CHECKMARK(module->bufferSize == 512);
		menu->addChild(resolution512);

		auto *resolution1024 = new ResolutionMenuItem();
		resolution1024->module = module;
		resolution1024->size = 1024;
		resolution1024->text = "Normal";
		resolution1024->rightText = CHECKMARK(module->bufferSize == 1024);
		menu->addChild(resolution1024);

		auto *resolution2048 = new ResolutionMenuItem();
		resolution2048->module = module;
		resolution2048->size = 2048;
		resolution2048->text = "Good";
		resolution2048->rightText = CHECKMARK(module->bufferSize == 2048);
		menu->addChild(resolution2048);

		auto *resolution4096 = new ResolutionMenuItem();
		resolution4096->module = module;
		resolution4096->size = 4096;
		resolution4096->text = "Ultra";
		resolution4096->rightText = CHECKMARK(module->bufferSize == 4096);
		menu->addChild(resolution4096);
	}
};

Model *modelOpsylloscope = createModel<Scope, ScopeWidget>("Opsylloscope");
