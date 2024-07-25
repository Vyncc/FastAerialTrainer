#include "pch.h"
#include "FastAerialTrainer.h"

std::string FastAerialTrainer::GetPluginName() {
	return "FastAerialTrainer";
}

void FastAerialTrainer::SetImGuiContext(uintptr_t ctx) {
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

static bool ColorPicker(const char* label, LinearColor& color) {
	LinearColor col = color / 255;
	bool retVal = ImGui::ColorEdit4(label, &col.R, ImGuiColorEditFlags_AlphaBar);
	color = col * 255;
	return retVal;
}

static bool ColorPickerWithoutAlpha(const char* label, LinearColor& color) {
	LinearColor col = color / 255;
	bool retVal = ImGui::ColorEdit3(label, &col.R);
	color = col * 255;
	return retVal;
}

static bool PercentageSlider(const char* label, float& value, float max = 1.f) {
	float x = value * 100;
	bool retVal = ImGui::SliderFloat(label, &x, 0, 100 * max, "%.1f %%");
	value = x / 100;
	return retVal;
}


// Render the plugin settings here
// This will show up in bakkesmod when the plugin is loaded at
//  f2 -> plugins -> FastAerialTrainer
void FastAerialTrainer::RenderSettings()
{
	if (ImGui::Checkbox("Enable Plugin", &PluginEnabled))
		cvarManager->getCvar(PLUGIN_ENABLED).setValue(PluginEnabled);

	if (ImGui::DragInt("GUI Position X", &GuiPosition.X))
		cvarManager->getCvar(GUI_POSITION_X).setValue(GuiPosition.X);

	if (ImGui::DragInt("GUI Position Y", &GuiPosition.Y))
		cvarManager->getCvar(GUI_POSITION_Y).setValue(GuiPosition.Y);

	if (ImGui::DragInt("GUI Size", &GuiSize))
		cvarManager->getCvar(GUI_SIZE).setValue(GuiSize);

	if (ColorPicker("Background Color", GuiColorBackground))
		cvarManager->getCvar(GUI_BACKGROUND_COLOR).setValue(GuiColorBackground);

	if (PercentageSlider("Color Preview Opacity", GuiColorPreviewOpacity, 0.5))
		cvarManager->getCvar(GUI_PREVIEW_OPACTIY).setValue(GuiColorPreviewOpacity);

	if (ColorPicker("Success Color", GuiColorSuccess))
		cvarManager->getCvar(GUI_COLOR_SUCCESS).setValue(GuiColorSuccess);

	if (ColorPicker("Warning Color", GuiColorWarning))
		cvarManager->getCvar(GUI_COLOR_WARNING).setValue(GuiColorWarning);

	if (ColorPicker("Failure Color", GuiColorFailure))
		cvarManager->getCvar(GUI_COLOR_FAILURE).setValue(GuiColorFailure);

	if (ImGui::DragInt("First Jump Hold - Highest Value", &JumpDuration_HighestValue, 1.f, 1, INT_MAX))
		cvarManager->getCvar(GUI_JUMP_MAX).setValue(JumpDuration_HighestValue);

	if (ImGui::DragInt("Time to Double Jump - Highest Value", &DoubleJumpDuration_HighestValue, 1.f, 1, INT_MAX))
		cvarManager->getCvar(GUI_DOUBLE_JUMP_MAX).setValue(DoubleJumpDuration_HighestValue);

	if (ImGui::Checkbox("Draw Pitch History", &GuiDrawPitchHistory))
		cvarManager->getCvar(GUI_DRAW_HISTORY).setValue(GuiDrawPitchHistory);

	if (ColorPickerWithoutAlpha("Pitch History Color - No Boost", GuiPitchHistoryColor))
		cvarManager->getCvar(GUI_COLOR_HISTORY).setValue(GuiPitchHistoryColor);

	if (ColorPickerWithoutAlpha("Pitch History Color - Boosting", GuiPitchHistoryColorBoost))
		cvarManager->getCvar(GUI_COLOR_HISTORY_BOOST).setValue(GuiPitchHistoryColorBoost);
}
