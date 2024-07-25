#include "pch.h"
#include "FastAerialTrainer.h"
#include "bakkesmod/wrappers/Engine/WorldInfoWrapper.h"
#include <sstream>


BAKKESMOD_PLUGIN(FastAerialTrainer, "FastAerialTrainer", plugin_version, PLUGINTYPE_FREEPLAY);

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

float FastAerialTrainer::GetCurrentTime() {
	ServerWrapper server = gameWrapper->GetCurrentGameState();
	if (!server) return 0;
	WorldInfoWrapper world = server.GetWorldInfo();
	if (!world) return 0;
	return world.GetTimeSeconds();
}

static std::string to_string(LinearColor col) {
	return "("
		+ std::to_string((int)col.R) + ","
		+ std::to_string((int)col.G) + ","
		+ std::to_string((int)col.B) + ","
		+ std::to_string((int)col.A) + ")";
}

void FastAerialTrainer::onLoad()
{
	_globalCvarManager = cvarManager;

	auto registerIntCvar = [this](std::string label, int& value)
		{
			cvarManager->registerCvar(label, std::to_string(value), "", false)
				.addOnValueChanged([&](std::string oldValue, CVarWrapper cvar) { value = cvar.getIntValue(); });
		};
	auto registerFloatCvar = [this](std::string label, float& value)
		{
			cvarManager->registerCvar(label, std::to_string(value), "", false)
				.addOnValueChanged([&](std::string oldValue, CVarWrapper cvar) { value = cvar.getFloatValue(); });
		};
	auto registerBoolCvar = [this](std::string label, bool& value)
		{
			cvarManager->registerCvar(label, std::to_string(value), "", false)
				.addOnValueChanged([&](std::string oldValue, CVarWrapper cvar) { value = cvar.getBoolValue(); });
		};
	auto registerColorCvar = [this](std::string label, LinearColor& value)
		{
			cvarManager->registerCvar(label, to_string(value), "", false)
				.addOnValueChanged([&](std::string oldValue, CVarWrapper cvar) { value = cvar.getColorValue(); });
		};

	registerBoolCvar(PLUGIN_ENABLED, PluginEnabled);
	registerIntCvar(GUI_POSITION_X, GuiPosition.X);
	registerIntCvar(GUI_POSITION_Y, GuiPosition.Y);
	registerIntCvar(GUI_SIZE, GuiSize);
	registerIntCvar(GUI_JUMP_MAX, JumpDuration_HighestValue);
	registerIntCvar(GUI_DOUBLE_JUMP_MAX, DoubleJumpDuration_HighestValue);
	registerFloatCvar(GUI_PREVIEW_OPACTIY, GuiColorPreviewOpacity);
	registerBoolCvar(GUI_DRAW_HISTORY, GuiDrawPitchHistory);
	registerColorCvar(GUI_BACKGROUND_COLOR, GuiColorBackground);
	registerColorCvar(GUI_COLOR_SUCCESS, GuiColorSuccess);
	registerColorCvar(GUI_COLOR_WARNING, GuiColorWarning);
	registerColorCvar(GUI_COLOR_FAILURE, GuiColorFailure);
	registerColorCvar(GUI_COLOR_HISTORY, GuiPitchHistoryColor);
	registerColorCvar(GUI_COLOR_HISTORY_BOOST, GuiPitchHistoryColorBoost);


	gameWrapper->RegisterDrawable(
		[this](CanvasWrapper canvas)
		{
			FastAerialTrainer::RenderCanvas(canvas);
		}
	);

	gameWrapper->HookEventWithCaller<CarWrapper>(
		"Function TAGame.Car_TA.SetVehicleInput",
		[this](CarWrapper car, void* params, std::string eventName)
		{
			FastAerialTrainer::OnTick(car);
		}
	);

	// Initial jump start
	gameWrapper->HookEvent(
		"Function CarComponent_Jump_TA.Active.BeginState",
		[this](std::string eventname)
		{
			float now = GetCurrentTime();

			holdFirstJumpStartTime = now;
			HoldingFirstJump = true;

			totalJumpTime = 0;
			HoldingJoystickBackDuration = 0;
			checkHoldingJoystickBack = true;
			inputHistory.clear();
			lastTickTime = now;
		}
	);

	// Jump released
	gameWrapper->HookEvent(
		"Function TAGame.Car_TA.OnJumpReleased",
		[this](std::string eventname)
		{
			if (HoldingFirstJump)
			{
				HoldingFirstJump = false;
				holdFirstJumpStopTime = GetCurrentTime();
			}
		}
	);

	// Double jump
	gameWrapper->HookEvent(
		"Function CarComponent_DoubleJump_TA.Active.BeginState",
		[this](std::string eventname)
		{
			// Only register the double jump if we didn't loose our flip or landed in between.
			if (checkHoldingJoystickBack) {
				DoubleJumpPressedTime = GetCurrentTime();
				totalJumpTime = DoubleJumpPressedTime - holdFirstJumpStartTime;
			}

			checkHoldingJoystickBack = false;
		}
	);
}

bool FastAerialTrainer::IsActive() {
	return PluginEnabled
		&& !gameWrapper->IsPaused()
		&& (gameWrapper->IsInFreeplay() || gameWrapper->IsInCustomTraining());
}

void FastAerialTrainer::OnTick(CarWrapper car)
{
	if (!IsActive())
		return;

	float now = GetCurrentTime();

	if (HoldingFirstJump)
		holdFirstJumpDuration = now - holdFirstJumpStartTime;
	else
		holdFirstJumpDuration = holdFirstJumpStopTime - holdFirstJumpStartTime;

	if (HoldingFirstJump)
		TimeBetweenFirstAndDoubleJump = 0;
	else if (DoubleJumpPressedTime > holdFirstJumpStopTime)
		TimeBetweenFirstAndDoubleJump = DoubleJumpPressedTime - holdFirstJumpStopTime;
	else if (checkHoldingJoystickBack)
		TimeBetweenFirstAndDoubleJump = now - holdFirstJumpStopTime;
	else
		TimeBetweenFirstAndDoubleJump = 0;

	// We either landed or have to land for another double jump. No need to record things further.
	if (!car.HasFlip() || (car.IsOnGround() && !car.GetbJumped()))
		checkHoldingJoystickBack = false;

	ControllerInput input = car.GetInput();
	if (checkHoldingJoystickBack)
	{
		float sensitivity = gameWrapper->GetSettings().GetGamepadSettings().AirControlSensitivity;
		float intensity = std::min(1.f, sensitivity * input.Pitch);
		float duration = now - lastTickTime;
		HoldingJoystickBackDuration += intensity * duration;
		lastTickTime = now;

		inputHistory.push_back({ intensity, (bool)input.HoldingBoost });
	}
}

static std::string toPrecision(float x, int precision) {
	std::stringstream stream;
	stream << std::fixed << std::setprecision(precision) << x;
	return stream.str();
}

void FastAerialTrainer::RenderCanvas(CanvasWrapper canvas)
{
	if (!IsActive())
		return;

	DrawBar(
		canvas, "Hold First Jump: ", holdFirstJumpDuration * 1000, JumpDuration_HighestValue,
		GuiPosition, BarSize(),
		GuiColorBackground, JumpDuration_RangeList
	);

	DrawBar(
		canvas, "Time to Double Jump: ", TimeBetweenFirstAndDoubleJump * 1000, DoubleJumpDuration_HighestValue,
		GuiPosition + Offset(), BarSize(),
		GuiColorBackground, DoubleJumpDuration_RangeList
	);

	canvas.SetColor(255, 255, 255, 255);
	canvas.SetPosition(GuiPosition + (Offset() * 2));
	float JoystickBackDurationPercentage = !totalJumpTime ? 0.f : 100.f * HoldingJoystickBackDuration / totalJumpTime;
	canvas.DrawString("Tilt Between Jumps: " + toPrecision(JoystickBackDurationPercentage, 1) + "%", FontSize(), FontSize());

	if (GuiDrawPitchHistory)
		DrawPitchHistory(canvas);
}

void FastAerialTrainer::DrawBar(
	CanvasWrapper& canvas, std::string text, float value, float maxValue,
	Vector2 barPos, Vector2 barSize,
	LinearColor backgroundColor, std::vector<Range>& colorRanges
)
{
	// Draw background
	canvas.SetPosition(barPos);
	canvas.SetColor(backgroundColor);
	canvas.FillBox(barSize);
	for (Range& range : colorRanges)
	{
		LinearColor preview = *range.color;
		preview.A *= GuiColorPreviewOpacity;
		canvas.SetColor(preview);
		float left = std::min(range.min / maxValue, 1.f);
		float width = std::min((range.max - range.min) / maxValue, 1.f - left);
		canvas.SetPosition(barPos + Vector2{ (int)(left * barSize.X), 0 });
		canvas.FillBox(Vector2{ (int)(width * barSize.X), barSize.Y });
	}

	// Draw colored bar
	canvas.SetPosition(barPos);
	for (Range& range : colorRanges)
	{
		if (range.min <= value && value <= range.max)
		{
			canvas.SetColor(*range.color);
		}
	}
	float result = std::min(1.f, value / maxValue);
	canvas.FillBox(Vector2{ (int)(barSize.X * result), barSize.Y });

	// Draw border
	canvas.SetColor(255, 255, 255, 255);
	canvas.SetPosition(barPos);
	canvas.DrawBox(barSize);
	for (Range& range : colorRanges)
	{
		canvas.SetColor(255, 255, 255, 255);
		float result = range.max / maxValue;
		if (result < 1.f) {
			canvas.SetPosition(barPos + Vector2{ (int)(result * barSize.X), 0 });
			canvas.FillBox(Vector2{ 2, barSize.Y });
		}
	}

	// Draw text
	canvas.SetColor(255, 255, 255, 255);
	canvas.SetPosition(barPos + Vector2{ 0, barSize.Y });
	canvas.DrawString(text + toPrecision(value, 1) + " ms", FontSize(), FontSize());
}

void FastAerialTrainer::DrawPitchHistory(CanvasWrapper& canvas)
{
	Vector2F offset = (Vector2F() + Offset()) * 2.6f + GuiPosition;
	canvas.SetColor(255, 255, 255, 127);
	float boxPadding = GuiSize / 200.f;
	canvas.SetPosition(offset - boxPadding);
	canvas.DrawBox(Vector2{ GuiSize, GuiSize / 10 } + 2 * boxPadding);

	int i = 0;
	int size = inputHistory.size();
	InputHistory prevInput;
	for (InputHistory currentInput : inputHistory)
	{
		if (i > 0 && currentInput.pitch >= 0 && prevInput.pitch >= 0)
		{
			Vector2F start = { (float)(i - 1) / size * GuiSize, (1 - prevInput.pitch) * GuiSize / 10 };
			Vector2F end = { (float)i / size * GuiSize, (1 - currentInput.pitch) * GuiSize / 10 };
			Vector2F base = start.Y > end.Y ? Vector2F{ end.X, start.Y } : Vector2F{ start.X, end.Y };
			canvas.SetColor(currentInput.boost ? GuiPitchHistoryColorBoost : GuiPitchHistoryColor);
			canvas.FillTriangle(base + offset, start + offset, end + offset); // `FillTriangle` ignores transparency.
			canvas.SetPosition(Vector2F{ start.X, std::max(start.Y, end.Y) } + offset);
			canvas.FillBox(Vector2F{ end.X - start.X, GuiSize / 10 - std::max(start.Y, end.Y) });
		}
		if (i % 12 == 0) {
			canvas.SetColor(200, 200, 200, 200);
			float x = (float)i / size * GuiSize;
			canvas.DrawLine(Vector2F{ x, 0 } + offset, Vector2F{ x, GuiSize / 10.f } + offset, GuiSize / 300.f);
		}
		prevInput = currentInput;
		i++;
	}
}

void FastAerialTrainer::onUnload()
{
}