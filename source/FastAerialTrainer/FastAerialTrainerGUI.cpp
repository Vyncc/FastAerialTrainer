#include "pch.h"
#include "FastAerialTrainer.h"

static bool ColorPicker(const char* label, LinearColor& color)
{
	LinearColor col = color / 255;
	bool retVal = ImGui::ColorEdit4(label, &col.R, ImGuiColorEditFlags_AlphaBar);
	color = col * 255;
	return retVal;
}

static bool PercentageSlider(const char* label, float& value, float max = 1.f)
{
	float x = value * 100;
	bool retVal = ImGui::SliderFloat(label, &x, 0, 100 * max, "%.1f %%");
	value = x / 100;
	return retVal;
}

static void SpacedSeparator()
{
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
}

void FastAerialTrainer::RenderSettings()
{
	if (ImGui::Checkbox("Enable Plugin", &PluginEnabled))
		cvarManager->getCvar(PLUGIN_ENABLED).setValue(PluginEnabled);

	if (ImGui::DragFloat("Record After Double Jump", &RecordingAfterDoubleJump, 0.005f, 0, FLT_MAX, "%.2f seconds"))
		cvarManager->getCvar(RECORD_AFTER_DOUBLE_JUMP).setValue(RecordingAfterDoubleJump);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Input recording duration after double jump input. Relevant for pitch and boost history graphs.\nDrag or ctrl-click to edit.");

	if (PercentageSlider("GUI Position X", GuiPositionRelative.X))
		cvarManager->getCvar(GUI_POSITION_RELATIVE_X).setValue(GuiPositionRelative.X);

	if (PercentageSlider("GUI Position Y", GuiPositionRelative.Y))
		cvarManager->getCvar(GUI_POSITION_RELATIVE_Y).setValue(GuiPositionRelative.Y);

	if (ImGui::SliderFloat("GUI Size", &GuiSize, 0, ScreenSize.X, "%.0f pixels"))
		cvarManager->getCvar(GUI_SIZE).setValue(GuiSize);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("For clearest text use multiples of 350 pixels.");

	if (ImGui::Checkbox("Show Initial Jump Timing\nHow long did you hold the jump input initially?", &GuiShowFirstJump))
		cvarManager->getCvar(GUI_SHOW_FIRST_JUMP).setValue(GuiShowFirstJump);

	if (ImGui::Checkbox("Show Double Jump Timing\nHow long did it take you to press the jump input again after releasing it?", &GuiShowDoubleJump))
		cvarManager->getCvar(GUI_SHOW_DOUBLE_JUMP).setValue(GuiShowDoubleJump);

	if (ImGui::Checkbox("Show Total Jump Timing\nHow long did it take you to press the jump input again after pressing it initially?", &GuiShowTotalJump))
		cvarManager->getCvar(GUI_SHOW_TOTAL_JUMP).setValue(GuiShowTotalJump);

	if (ImGui::Checkbox("Show Pitch Up Amount While Jumping", &GuiShowPitchAmount))
		cvarManager->getCvar(GUI_SHOW_PITCH_AMOUNT).setValue(GuiShowPitchAmount);

	if (ImGui::Checkbox("Draw Pitch History", &GuiShowPitchHistory))
		cvarManager->getCvar(GUI_DRAW_PITCH_HISTORY).setValue(GuiShowPitchHistory);

	if (ImGui::Checkbox("Show Pitch Down in History", &GuiShowPitchDownInHistory))
		cvarManager->getCvar(GUI_SHOW_PITCH_DOWN_IN_HISTORY).setValue(GuiShowPitchDownInHistory);

	if (ImGui::Checkbox("Draw Boost History", &GuiShowBoostHistory))
		cvarManager->getCvar(GUI_DRAW_BOOST_HISTORY).setValue(GuiShowBoostHistory);

	if (ImGui::Checkbox("Show First Input Warning in Custom Training", &GuiShowFirstInputWarning))
		cvarManager->getCvar(GUI_SHOW_FIRST_INPUT_WARNING).setValue(GuiShowFirstInputWarning);

	if (ImGui::DragInt("Add a marker every n-th game-tick", &GuiTicksPerMarker, 0.02f, 0, 120, ("%d ticks (approx. " + std::to_string((int)std::round(GuiTicksPerMarker / 120.f * 1000.f)) + " ms)").c_str()))
		cvarManager->getCvar(GUI_TICKS_PER_MARKER).setValue(GuiTicksPerMarker);
	if(ImGui::IsItemHovered())
		ImGui::SetTooltip("Set to 0 to disable.\nDrag or ctrl-click to edit.");

	SpacedSeparator();

	ImGui::Text("Theme / Colors");
	ImGui::Spacing();

	if (ColorPicker("Border and Text Color", GuiColorBorder))
		cvarManager->getCvar(GUI_BORDER_COLOR).setValue(GuiColorBorder);

	if (ColorPicker("Bar Background Color", GuiColorBackground))
		cvarManager->getCvar(GUI_BACKGROUND_COLOR).setValue(GuiColorBackground);

	if (ColorPicker("Backdrop Color", GuiColorBackdrop))
		cvarManager->getCvar(GUI_BACKDROP_COLOR).setValue(GuiColorBackdrop);

	if (PercentageSlider("Color Preview Opacity", GuiColorPreviewOpacity, 0.5))
		cvarManager->getCvar(GUI_PREVIEW_OPACTIY).setValue(GuiColorPreviewOpacity);

	if (ColorPicker("Success Color", GuiColorSuccess))
		cvarManager->getCvar(GUI_COLOR_SUCCESS).setValue(GuiColorSuccess);

	if (ColorPicker("Warning Color", GuiColorWarning))
		cvarManager->getCvar(GUI_COLOR_WARNING).setValue(GuiColorWarning);

	if (ColorPicker("Failure Color", GuiColorFailure))
		cvarManager->getCvar(GUI_COLOR_FAILURE).setValue(GuiColorFailure);

	if (ColorPicker("Pitch/Boost History Color", GuiPitchHistoryColor))
		cvarManager->getCvar(GUI_COLOR_HISTORY).setValue(GuiPitchHistoryColor);

	SpacedSeparator();

	ImGui::PushID("InitialJump");
	ImGui::Text("Initial Jump Timing");
	ImGui::Spacing();
	RenderRangePicker(
		JumpDurationRanges,
		GUI_JUMP_RANGES,
		{
			"Failure Low",
			"Warning Low",
			"Success Low",
			"Success High",
			"Warning High",
			"Failure High"
		}
	);
	ImGui::PopID();

	SpacedSeparator();

	ImGui::PushID("DoubleJump");
	ImGui::Text("Double Jump Timing");
	ImGui::Spacing();
	RenderRangePicker(
		DoubleJumpDurationRanges,
		GUI_DOUBLE_JUMP_RANGES,
		{
			"Success Low",
			"Success High",
			"Warning High",
			"Failure High"
		}
	);
	ImGui::PopID();

	SpacedSeparator();

	ImGui::PushID("TotalJump");
	ImGui::Text("Total Jump Timing");
	ImGui::Spacing();
	RenderRangePicker(
		TotalJumpDurationRanges,
		GUI_TOTAL_JUMP_RANGES,
		{
			"Success Low",
			"Success High",
			"Warning High",
			"Failure High"
		}
	);
	ImGui::PopID();
}

void FastAerialTrainer::RenderRangePicker(RangeList& rangeList, const char* cvar, std::vector<const char*> labels)
{
	float speed = 0.1f;
	const char* format = "%.1f ms";
	float width = 200;
	float spacing = 20;

	auto values = rangeList.GetValues();

	ImGui::PushItemWidth(width);

	for (int i = 0; i < labels.size(); i++)
	{
		float value = values[i];
		float min = i - 1 < 0 ? 0 : values[i - 1];
		float max = i + 1 >= values.size() ? FLT_MAX : values[i + 1];

		if (ImGui::DragFloat(labels[i], &value, speed, min, max, format))
		{
			rangeList.UpdateValue(i, value);
			cvarManager->getCvar(cvar).setValue(rangeList.ValuesToString());
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Drag or ctrl-click to edit.");
	}

	ImGui::Spacing();
	if (ImGui::Button("Reset to default"))
		cvarManager->getCvar(cvar).ResetToDefault();

	ImGui::PopItemWidth();
}