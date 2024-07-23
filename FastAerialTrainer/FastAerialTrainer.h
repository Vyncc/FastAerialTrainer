#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

struct Range
{
	int min;
	int max;
	LinearColor* color;
};

struct InputHistory
{
	float pitch;
	bool boost;
};

class FastAerialTrainer : public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginSettingsWindow
{
	// Measuring

	bool HoldingFirstJump = false;
	float holdFirstJumpStartTime;
	float holdFirstJumpStopTime;
	float holdFirstJumpDuration;

	float DoubleJumpPressedTime;
	float TimeBetweenFirstAndDoubleJump;

	bool checkHoldingJoystickBack = false;
	float lastTickTime;
	float HoldingJoystickBackDuration;
	std::vector<InputHistory> inputHistory;

	float totalJumpTime;


	// Styling

	Vector2 GuiPosition = { 570, 12 };
	int GuiSize = 825;
	Vector2 BarSize() { return { GuiSize, GuiSize / 24 }; }
	Vector2 Offset() { return { 0, GuiSize / 10 }; }
	float FontSize() { return GuiSize / 350.f; }
	float GuiColorPreviewOpacity = 0.2;
	LinearColor GuiColorBackground = LinearColor(255, 255, 255, 150);
	LinearColor GuiColorSuccess = LinearColor(0, 0, 255, 210);
	LinearColor GuiColorWarning = LinearColor(255, 255, 0, 210);
	LinearColor GuiColorFailure = LinearColor(255, 0, 0, 210);
	std::vector<Range> JumpDuration_RangeList =
	{
		Range{ 0, 180, &GuiColorFailure },
		Range{ 180, 195, &GuiColorWarning },
		Range{ 195, 225, &GuiColorSuccess },
		Range{ 225, 260, &GuiColorWarning },
		Range{ 260, INT_MAX, &GuiColorFailure }
	};
	int JumpDuration_HighestValue = 300;
	std::vector<Range> DoubleJumpDuration_RangeList =
	{
		Range{ 0, 75, &GuiColorSuccess },
		Range{ 75, 110, &GuiColorWarning },
		Range{ 110, INT_MAX, &GuiColorFailure }
	};
	int DoubleJumpDuration_HighestValue = 130;
	LinearColor GuiPitchHistoryColor = LinearColor(240, 240, 240, 255);
	LinearColor GuiPitchHistoryColorBoost = LinearColor(240, 80, 80, 255);
	bool GuiDrawPitchHistory = true;


	// Methods

	float GetCurrentTime();
	void OnTick(CarWrapper car, ControllerInput input);

	void RenderCanvas(CanvasWrapper canvas);
	void DrawBar(CanvasWrapper& canvas, std::string text, float value, float maxValue, Vector2 barPos, Vector2 barSize, LinearColor backgroundColor, std::vector<Range>& colorRanges);
	void DrawPitchHistory(CanvasWrapper& canvas);

	virtual void onLoad();
	virtual void onUnload();

	void RenderSettings() override;
	std::string GetPluginName() override;
	void SetImGuiContext(uintptr_t ctx) override;
};
