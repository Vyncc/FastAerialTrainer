#pragma once

#include "GuiBase.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "PersistentStorage.h"
#include "RangeList.h"
#include "TransparentTriangle.h"

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

constexpr auto PLUGIN_ENABLED = "fast_aerial_trainer_enabled";
constexpr auto RECORD_AFTER_DOUBLE_JUMP = "fast_aerial_trainer_record_after_double_jump";
constexpr auto GUI_POSITION_RELATIVE_X = "fast_aerial_trainer_gui_pos_rel_x";
constexpr auto GUI_POSITION_RELATIVE_Y = "fast_aerial_trainer_gui_pos_rel_y";
constexpr auto GUI_SIZE = "fast_aerial_trainer_gui_size";
constexpr auto GUI_BORDER_COLOR = "fast_aerial_trainer_gui_border_color";
constexpr auto GUI_BACKGROUND_COLOR = "fast_aerial_trainer_gui_background_color";
constexpr auto GUI_BACKDROP_COLOR = "fast_aerial_trainer_gui_backdrop_color";
constexpr auto GUI_PREVIEW_OPACTIY = "fast_aerial_trainer_gui_preview_opacity";
constexpr auto GUI_COLOR_SUCCESS = "fast_aerial_trainer_gui_color_success";
constexpr auto GUI_COLOR_WARNING = "fast_aerial_trainer_gui_color_warning";
constexpr auto GUI_COLOR_FAILURE = "fast_aerial_trainer_gui_color_failure";
constexpr auto GUI_COLOR_HISTORY = "fast_aerial_trainer_gui_color_history";
constexpr auto GUI_JUMP_RANGES = "fast_aerial_trainer_gui_jump_ranges";
constexpr auto GUI_DOUBLE_JUMP_RANGES = "fast_aerial_trainer_gui_double_jump_ranges";
constexpr auto GUI_SHOW_FIRST_JUMP = "fast_aerial_trainer_gui_show_first_jump";
constexpr auto GUI_SHOW_DOUBLE_JUMP = "fast_aerial_trainer_gui_show_double_jump";
constexpr auto GUI_SHOW_PITCH_AMOUNT = "fast_aerial_trainer_gui_show_pitch_amount";
constexpr auto GUI_DRAW_PITCH_HISTORY = "fast_aerial_trainer_gui_draw_pitch_history";
constexpr auto GUI_SHOW_PITCH_DOWN_IN_HISTORY = "fast_aerial_trainer_gui_show_pitch_down_in_history";
constexpr auto GUI_DRAW_BOOST_HISTORY = "fast_aerial_trainer_gui_draw_boost_history";
constexpr auto GUI_SHOW_FIRST_INPUT_WARNING = "fast_aerial_trainer_gui_first_input_warning";
constexpr auto GUI_TICKS_PER_MARKER = "fast_aerial_trainer_gui_ticks_per_marker";

struct InputHistoryItem
{
	float pitch;
	bool boost;
	bool jumped;
};

class FastAerialTrainer : public BakkesMod::Plugin::BakkesModPlugin, public SettingsWindowBase
{
	std::shared_ptr<PersistentStorage> persistentStorage;
	std::shared_ptr<TransparentTriangle> transparentTriangle;


	// Settings

	bool PluginEnabled = true;
	bool IsInReplay = false;
	float RecordingAfterDoubleJump = 0.2;


	// Measuring

	bool HoldingFirstJump = false;
	float HoldFirstJumpStartTime = 0;
	float HoldFirstJumpStopTime = 0;
	float HoldFirstJumpDuration = 0;

	bool DoubleJumpPossible = false;
	float DoubleJumpPressedTime = 0;
	float TimeBetweenFirstAndDoubleJump = 0;
	float PitchUpBetweenJumps = 0;
	float TicksBetweenJumps = 0;

	std::vector<InputHistoryItem> InputHistory;

	float TrainingStartTime = 0;


	// Styling

	Vector2F GuiPositionRelative = { 0.5, 0.01 };
	Vector2 ScreenSize = { 1920, 1080 };
	float GuiSize = 700;
	float GuiHeight = 0; // dynamically computed
	Vector2F BarSize() { return { GuiSize, GuiSize / 24.f }; }
	Vector2F Offset() { return { 0, GuiSize / 10.f }; }
	float FontSize() { return GuiSize / 350.f; }
	Vector2F GuiPosition()
	{
		return {
			std::lerp(0.f, ScreenSize.X - GuiSize, GuiPositionRelative.X),
			std::lerp(0.f, ScreenSize.Y - 0.25f * GuiSize, GuiPositionRelative.Y)
		};
	}
	float GuiColorPreviewOpacity = 0.2f;
	LinearColor GuiColorBorder = LinearColor(255, 255, 255, 255);
	LinearColor GuiColorBackground = LinearColor(255, 255, 255, 127);
	LinearColor GuiColorBackdrop = LinearColor(127, 127, 127, 0);
	LinearColor GuiColorSuccess = LinearColor(0, 0, 255, 210);
	LinearColor GuiColorWarning = LinearColor(255, 255, 0, 210);
	LinearColor GuiColorFailure = LinearColor(255, 0, 0, 210);
	RangeList JumpDurationRanges = RangeList(
		{ 0, 180, 195, 225, 260, 300 },
		{
			 &GuiColorFailure,
			 &GuiColorWarning,
			 &GuiColorSuccess,
			 &GuiColorWarning,
			 &GuiColorFailure
		}
	);
	RangeList DoubleJumpDurationRanges = RangeList(
		{ 0, 75, 110, 130 },
		{
			 &GuiColorSuccess,
			 &GuiColorWarning,
			 &GuiColorFailure
		}
	);
	LinearColor GuiPitchHistoryColor = LinearColor(255, 255, 255, 239);

	bool GuiShowFirstJump = true;
	bool GuiShowDoubleJump = true;
	bool GuiShowPitchAmount = true;
	bool GuiShowPitchHistory = true;
	bool GuiShowPitchDownInHistory = false;
	bool GuiShowBoostHistory = true;
	bool GuiShowFirstInputWarning = true;
	int GuiTicksPerMarker = 0;


	// Methods

	bool IsActive();
	bool IsLocalCar(CarWrapper car);

	float GetCurrentTime();
	void OnTick(CarWrapper car, ControllerInput* input);

	void RenderCanvas(CanvasWrapper canvas);
	void DrawBar(CanvasWrapper& canvas, std::string text, float value, Vector2F barPos, Vector2F barSize, LinearColor backgroundColor, RangeList& colorRanges);
	void DrawPitchHistory(CanvasWrapper& canvas, Vector2F position);
	void DrawBoostHistory(CanvasWrapper& canvas, Vector2F position);

	virtual void onLoad();
	virtual void onUnload();

	void RenderSettings() override;
	void RenderRangePicker(RangeList& rangeList, const char* cvar, std::vector<const char*> labels);
};
