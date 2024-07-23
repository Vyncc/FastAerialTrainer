#include "pch.h"
#include "FastAerialTrainer.h"

std::string FastAerialTrainer::GetPluginName() {
	return "FastAerialTrainer";
}

void FastAerialTrainer::SetImGuiContext(uintptr_t ctx) {
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

static void ColorPicker(const char* label, LinearColor* color) {
	LinearColor col = (*color) / 255;
	ImGui::ColorEdit4(label, &col.R, ImGuiColorEditFlags_AlphaBar);
	*color = col * 255;
}

static void ColorPickerWithoutAlpha(const char* label, LinearColor* color) {
	LinearColor col = (*color) / 255;
	ImGui::ColorEdit3(label, &col.R);
	*color = col * 255;
}

static void PercentageSlider(const char* label, float* value, float max = 1.f) {
	float x = (*value) * 100;
	ImGui::SliderFloat(label, &x, 0, 100 * max, "%.1f %%");
	*value = x / 100;
}


// Render the plugin settings here
// This will show up in bakkesmod when the plugin is loaded at
//  f2 -> plugins -> FastAerialTrainer
void FastAerialTrainer::RenderSettings()
{
	ImGui::DragInt("GUI Position X", &GuiPosition.X);
	ImGui::DragInt("GUI Position Y", &GuiPosition.Y);
	ImGui::DragInt("GUI Size", &GuiSize);
	ColorPicker("Background Color", &GuiColorBackground);
	PercentageSlider("Color Preview Opacity", &GuiColorPreviewOpacity, 0.5);
	ColorPicker("Success Color", &GuiColorSuccess);
	ColorPicker("Warning Color", &GuiColorWarning);
	ColorPicker("Failure Color", &GuiColorFailure);
	ImGui::DragInt("First Jump Hold - Highest Value", &JumpDuration_HighestValue, 1.f, 1, INT_MAX);
	ImGui::DragInt("Time to Double Jump - Highest Value", &DoubleJumpDuration_HighestValue, 1.f, 1, INT_MAX);
	ImGui::Checkbox("Draw Pitch History", &GuiDrawPitchHistory);
	ColorPickerWithoutAlpha("Pitch History Color - Boosting", &GuiPitchHistoryColorBoost);
	ColorPickerWithoutAlpha("Pitch History Color - No Boost", &GuiPitchHistoryColor);
}
