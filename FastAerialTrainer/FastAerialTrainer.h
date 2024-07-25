#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

constexpr auto PLUGIN_ENABLED = "fast_aerial_trainer_enabled";
constexpr auto GUI_POSITION_X = "fast_aerial_trainer_gui_pos_x";
constexpr auto GUI_POSITION_Y = "fast_aerial_trainer_gui_pos_y";
constexpr auto GUI_SIZE = "fast_aerial_trainer_gui_size";
constexpr auto GUI_BACKGROUND_COLOR = "fast_aerial_trainer_gui_background_color";
constexpr auto GUI_PREVIEW_OPACTIY = "fast_aerial_trainer_gui_preview_opactiy";
constexpr auto GUI_COLOR_SUCCESS = "fast_aerial_trainer_gui_color_success";
constexpr auto GUI_COLOR_WARNING = "fast_aerial_trainer_gui_color_warning";
constexpr auto GUI_COLOR_FAILURE = "fast_aerial_trainer_gui_color_failure";
constexpr auto GUI_JUMP_MAX = "fast_aerial_trainer_gui_jump_max";
constexpr auto GUI_DOUBLE_JUMP_MAX = "fast_aerial_trainer_gui_double_jump_max";
constexpr auto GUI_DRAW_HISTORY = "fast_aerial_trainer_gui_draw_history";
constexpr auto GUI_COLOR_HISTORY = "fast_aerial_trainer_gui_color_history";
constexpr auto GUI_COLOR_HISTORY_BOOST = "fast_aerial_trainer_gui_color_history_boost";

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
	bool PluginEnabled = true;

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

	Vector2 GuiPosition = { 610, 16 };
	int GuiSize = 700;
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

	bool IsActive();

	float GetCurrentTime();
	void OnTick(CarWrapper car);

	void RenderCanvas(CanvasWrapper canvas);
	void DrawBar(CanvasWrapper& canvas, std::string text, float value, float maxValue, Vector2 barPos, Vector2 barSize, LinearColor backgroundColor, std::vector<Range>& colorRanges);
	void DrawPitchHistory(CanvasWrapper& canvas);

	virtual void onLoad();
	virtual void onUnload();

	void RenderSettings() override;
	std::string GetPluginName() override;
	void SetImGuiContext(uintptr_t ctx) override;
};
