#include "pch.h"
#include "FastAerialTrainer.h"

#include <sstream>
#include <set>


BAKKESMOD_PLUGIN(FastAerialTrainer, "FastAerialTrainer", plugin_version, PLUGINTYPE_FREEPLAY);

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

float FastAerialTrainer::GetCurrentTime()
{
	ServerWrapper server = gameWrapper->GetCurrentGameState();
	if (!server) return 0;

	return server.GetSecondsElapsed();
}

static std::string to_string(LinearColor col)
{
	return "("
		+ std::to_string((int)col.R) + ","
		+ std::to_string((int)col.G) + ","
		+ std::to_string((int)col.B) + ","
		+ std::to_string((int)col.A) + ")";
}

void FastAerialTrainer::onLoad()
{
	// This line is required for `LOG` to work and must be before any use of `LOG()`.
	_globalCvarManager = cvarManager;

	persistentStorage = std::make_shared<PersistentStorage>(this, "fast_aerial_trainer", true, true);

	auto triangleImage = std::make_shared<ImageWrapper>(gameWrapper->GetDataFolder() / "FastAerialTrainer" / "triangle.png");
	transparentTriangle = std::make_shared<TransparentTriangle>(triangleImage);

	auto registerIntCvar = [this](std::string label, int& value)
		{
			persistentStorage->RegisterPersistentCvar(label, std::to_string(value), "", true)
				.addOnValueChanged([&](std::string oldValue, CVarWrapper cvar) { value = cvar.getIntValue(); });
		};
	auto registerFloatCvar = [this](std::string label, float& value)
		{
			persistentStorage->RegisterPersistentCvar(label, std::to_string(value), "", true)
				.addOnValueChanged([&](std::string oldValue, CVarWrapper cvar) { value = cvar.getFloatValue(); });
		};
	auto registerPercentCvar = [this](std::string label, float& value)
		{
			persistentStorage->RegisterPersistentCvar(label, std::to_string(value), "", true, true, 0.0f, true, 1.0f)
				.addOnValueChanged([&](std::string oldValue, CVarWrapper cvar) { value = cvar.getFloatValue(); });
		};
	auto registerBoolCvar = [this](std::string label, bool& value)
		{
			persistentStorage->RegisterPersistentCvar(label, std::to_string(value), "", true)
				.addOnValueChanged([&](std::string oldValue, CVarWrapper cvar) { value = cvar.getBoolValue(); });
		};
	auto registerColorCvar = [this](std::string label, LinearColor& value)
		{
			persistentStorage->RegisterPersistentCvar(label, to_string(value), "", true)
				.addOnValueChanged([&](std::string oldValue, CVarWrapper cvar) { value = cvar.getColorValue(); });
		};
	auto registerRangeListCvar = [this](std::string label, RangeList& rangeList)
		{
			persistentStorage->RegisterPersistentCvar(label, rangeList.ValuesToString(), "", true)
				.addOnValueChanged([&](std::string oldValue, CVarWrapper cvar)
					{
						auto values = RangeList::SplitString(cvar.getStringValue());
						rangeList.UpdateValues(values);
					}
				);
		};

	registerBoolCvar(PLUGIN_ENABLED, PluginEnabled);
	registerFloatCvar(RECORD_AFTER_DOUBLE_JUMP, RecordingAfterDoubleJump);
	registerPercentCvar(GUI_POSITION_RELATIVE_X, GuiPositionRelative.X);
	registerPercentCvar(GUI_POSITION_RELATIVE_Y, GuiPositionRelative.Y);
	registerFloatCvar(GUI_SIZE, GuiSize);
	registerPercentCvar(GUI_PREVIEW_OPACTIY, GuiColorPreviewOpacity);
	registerBoolCvar(GUI_SHOW_FIRST_JUMP, GuiShowFirstJump);
	registerBoolCvar(GUI_SHOW_DOUBLE_JUMP, GuiShowDoubleJump);
	registerBoolCvar(GUI_SHOW_TOTAL_JUMP, GuiShowTotalJump);
	registerBoolCvar(GUI_SHOW_PITCH_AMOUNT, GuiShowPitchAmount);
	registerBoolCvar(GUI_DRAW_PITCH_HISTORY, GuiShowPitchHistory);
	registerBoolCvar(GUI_SHOW_PITCH_DOWN_IN_HISTORY, GuiShowPitchDownInHistory);
	registerBoolCvar(GUI_DRAW_BOOST_HISTORY, GuiShowBoostHistory);
	registerBoolCvar(GUI_SHOW_FIRST_INPUT_WARNING, GuiShowFirstInputWarning);
	registerIntCvar(GUI_TICKS_PER_MARKER, GuiTicksPerMarker);
	registerColorCvar(GUI_BORDER_COLOR, GuiColorBorder);
	registerColorCvar(GUI_BACKGROUND_COLOR, GuiColorBackground);
	registerColorCvar(GUI_BACKDROP_COLOR, GuiColorBackdrop);
	registerColorCvar(GUI_COLOR_SUCCESS, GuiColorSuccess);
	registerColorCvar(GUI_COLOR_WARNING, GuiColorWarning);
	registerColorCvar(GUI_COLOR_FAILURE, GuiColorFailure);
	registerColorCvar(GUI_COLOR_HISTORY, GuiPitchHistoryColor);
	registerRangeListCvar(GUI_JUMP_RANGES, JumpDurationRanges);
	registerRangeListCvar(GUI_DOUBLE_JUMP_RANGES, DoubleJumpDurationRanges);
	registerRangeListCvar(GUI_TOTAL_JUMP_RANGES, TotalJumpDurationRanges);

	gameWrapper->RegisterDrawable(
		[this](CanvasWrapper canvas)
		{
			if (!IsActive()) return;

			FastAerialTrainer::RenderCanvas(canvas);
		}
	);

	gameWrapper->HookEventWithCaller<CarWrapper>(
		"Function TAGame.Car_TA.SetVehicleInput",
		[this](CarWrapper car, void* params, std::string eventName)
		{
			if (IsInReplay || !IsActive()) return;
			if (!IsLocalCar(car)) return;

			auto input = static_cast<ControllerInput*>(params);
			if (!input) return;

			FastAerialTrainer::OnTick(car, input);
		}
	);

	// Initial jump start
	gameWrapper->HookEventWithCaller<CarComponentWrapper>(
		"Function CarComponent_Jump_TA.Active.BeginState",
		[this](CarComponentWrapper component, void* params, std::string eventName)
		{
			if (IsInReplay || !IsActive()) return;
			if (!IsLocalCar(component.GetCar())) return;

			float now = GetCurrentTime();

			HoldingFirstJump = true;
			HoldFirstJumpStartTime = now;
			HoldFirstJumpStopTime = 0;
			HoldFirstJumpDuration = 0;

			DoubleJumpPossible = true;
			DoubleJumpPressedTime = 0;
			TimeBetweenFirstAndDoubleJump = 0;
			TicksBetweenJumps = 0;
			PitchUpBetweenJumps = 0;

			InputHistory.clear();
		}
	);

	auto OnSecondJump = [this](CarComponentWrapper component)
		{
			if (IsInReplay || !IsActive()) return;
			if (!IsLocalCar(component.GetCar())) return;

			// Only register the double jump if we didn't loose our flip or landed in between.
			if (DoubleJumpPossible)
				DoubleJumpPressedTime = GetCurrentTime();

			DoubleJumpPossible = false;
		};

	// Double jump
	gameWrapper->HookEventWithCaller<CarComponentWrapper>(
		"Function CarComponent_DoubleJump_TA.Active.BeginState",
		[this, OnSecondJump](CarComponentWrapper component, ...)
		{
			OnSecondJump(component);
		}
	);

	// Dodge
	gameWrapper->HookEventWithCaller<CarComponentWrapper>(
		"Function CarComponent_Dodge_TA.Active.BeginState",
		[this, OnSecondJump](CarComponentWrapper component, ...)
		{
			// Treat dodging the same as double-jumping, since players might dodge by accident.
			OnSecondJump(component);
		}
	);

	// Jump released
	gameWrapper->HookEventWithCaller<CarWrapper>(
		"Function TAGame.Car_TA.OnJumpReleased",
		[this](CarWrapper car, void* params, std::string eventName)
		{
			if (IsInReplay || !IsActive()) return;
			if (!IsLocalCar(car)) return;

			if (HoldingFirstJump)
			{
				HoldingFirstJump = false;
				HoldFirstJumpStopTime = GetCurrentTime();
			}
		}
	);

	// Are we in a replay?
	gameWrapper->HookEvent(
		"Function GameEvent_Soccar_TA.ReplayPlayback.BeginState",
		[this](...) { IsInReplay = true; }
	);
	gameWrapper->HookEvent(
		"Function GameEvent_Soccar_TA.ReplayPlayback.EndState",
		[this](...) { IsInReplay = false; }
	);

	// Freeplay or training start/reset
	gameWrapper->HookEvent(
		"Function GameEvent_Soccar_TA.Countdown.BeginState",
		[this](...)
		{
			HoldingFirstJump = false;
			DoubleJumpPossible = false;
		}
	);

	// Custom training attempt started
	gameWrapper->HookEvent(
		"Function TAGame.TrainingEditorMetrics_TA.TrainingShotAttempt",
		[this](...)
		{
			TrainingStartTime = GetCurrentTime();
		}
	);
}

bool FastAerialTrainer::IsActive()
{
	return PluginEnabled
		&& !gameWrapper->IsPaused()
		&& (gameWrapper->IsInFreeplay() || gameWrapper->IsInCustomTraining());
}

bool FastAerialTrainer::IsLocalCar(CarWrapper car)
{
	CarWrapper localCar = gameWrapper->GetLocalCar();
	return car.memory_address == localCar.memory_address;
}

void FastAerialTrainer::OnTick(CarWrapper car, ControllerInput* input)
{
	float now = GetCurrentTime();

	if (HoldingFirstJump)
		HoldFirstJumpDuration = now - HoldFirstJumpStartTime;
	else if (HoldFirstJumpStopTime > 0)
		HoldFirstJumpDuration = HoldFirstJumpStopTime - HoldFirstJumpStartTime;

	if (HoldingFirstJump)
		TimeBetweenFirstAndDoubleJump = 0;
	else if (DoubleJumpPressedTime >= HoldFirstJumpStopTime)
		TimeBetweenFirstAndDoubleJump = DoubleJumpPressedTime - HoldFirstJumpStopTime;
	else if (DoubleJumpPossible)
		TimeBetweenFirstAndDoubleJump = now - HoldFirstJumpStopTime;

	if (HoldingFirstJump || DoubleJumpPressedTime >= HoldFirstJumpStopTime || DoubleJumpPossible)
		TotalJumpDuration = HoldFirstJumpDuration + TimeBetweenFirstAndDoubleJump;

	// We either landed or have to land for another double jump. No need to record things further.
	if (!car.HasFlip() || (car.IsOnGround() && !car.GetbJumped()))
		DoubleJumpPossible = false;

	auto InAfterDoubleJumpRecording = DoubleJumpPressedTime <= now
		&& now <= DoubleJumpPressedTime + RecordingAfterDoubleJump;

	if (DoubleJumpPossible || InAfterDoubleJumpRecording)
	{
		float sensitivity = gameWrapper->GetSettings().GetGamepadSettings().AirControlSensitivity;
		float intensity = std::clamp(sensitivity * input->Pitch, -1.f, 1.f);

		InputHistory.push_back({ intensity, (bool)input->HoldingBoost, (bool)input->Jumped });

		if (DoubleJumpPossible)
		{
			TicksBetweenJumps += 1;
			PitchUpBetweenJumps += intensity;
		}
	}
}

static std::string toPrecision(float x, int precision)
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(precision) << x;
	return stream.str();
}

void FastAerialTrainer::RenderCanvas(CanvasWrapper canvas)
{
	ScreenSize = canvas.GetSize();

	Vector2F position = GuiPosition();

	{
		Vector2F margin = { 0.02f, 0.04f };
		Vector2F size = Vector2F{ GuiSize, GuiHeight };
		canvas.SetPosition(position - (size * margin));
		canvas.SetColor(GuiColorBackdrop);
		canvas.FillBox(size * ((margin * 2.f) + 1.f));
	}

	if (GuiShowFirstJump)
	{
		DrawBar(
			canvas, "Hold First Jump: ", HoldFirstJumpDuration * 1000,
			position, BarSize(),
			GuiColorBackground, JumpDurationRanges
		);
		position += Offset();
	}

	if (GuiShowDoubleJump)
	{
		DrawBar(
			canvas, "Time to Double Jump: ", TimeBetweenFirstAndDoubleJump * 1000,
			position, BarSize(),
			GuiColorBackground, DoubleJumpDurationRanges
		);
		position += Offset();
	}

	if (GuiShowTotalJump)
	{
		DrawBar(
			canvas, "Total Jump Duration: ", TotalJumpDuration * 1000,
			position, BarSize(),
			GuiColorBackground, TotalJumpDurationRanges
		);
		position += Offset();
	}

	if (GuiShowPitchAmount)
	{
		canvas.SetColor(GuiColorBorder);
		canvas.SetPosition(position);
		float PitchUpBetweenJumpsAmount = TicksBetweenJumps == 0 ? 0 : PitchUpBetweenJumps / TicksBetweenJumps;
		canvas.DrawString("Pitch Up Between Jumps: " + toPrecision(100 * PitchUpBetweenJumpsAmount, 1) + "%", FontSize(), FontSize());

		position += Offset() * 0.6f;
	}

	if (GuiShowPitchHistory)
	{
		DrawPitchHistory(canvas, position);
		position += Offset() * 1.2f;
	}

	if (GuiShowBoostHistory)
	{
		DrawBoostHistory(canvas, position);
		position += Offset() * 0.6f;
	}

	if (GuiShowFirstInputWarning && gameWrapper->IsInCustomTraining() && TrainingStartTime < HoldFirstJumpStartTime)
	{
		canvas.SetColor(GuiColorBorder);
		canvas.SetPosition(position);
		canvas.DrawString("Jump was not first input!", FontSize(), FontSize());

		position += Offset() * 0.6f;
	}

	GuiHeight = position.Y - GuiPosition().Y;
}

void FastAerialTrainer::DrawBar(
	CanvasWrapper& canvas, std::string text, float value,
	Vector2F barPos, Vector2F barSize,
	LinearColor backgroundColor, RangeList& colorRanges
)
{
	if (colorRanges.IsEmpty())
		return;

	float minValue = colorRanges.GetTotalMin();
	float maxValue = colorRanges.GetTotalMax();
	auto valueToPosition = [&](float value)
		{
			auto ratio = (value - minValue) / (maxValue - minValue);
			return barSize.X * std::clamp(ratio, 0.f, 1.f);
		};

	// Draw background
	canvas.SetPosition(barPos);
	canvas.SetColor(backgroundColor);
	canvas.FillBox(barSize);

	for (Range& range : colorRanges.GetRanges())
	{
		LinearColor preview = *range.color;
		preview.A *= GuiColorPreviewOpacity;
		canvas.SetColor(preview);

		float left = valueToPosition(range.min);
		float right = valueToPosition(range.max);
		float width = right - left;
		canvas.SetPosition(barPos + Vector2F{ left, 0 });
		canvas.FillBox(Vector2F{ width, barSize.Y });
	}

	// Draw colored bar
	canvas.SetColor(*colorRanges.GetColorForValue(value));
	canvas.SetPosition(barPos);
	canvas.FillBox(Vector2F{ valueToPosition(value), barSize.Y });

	// Draw separators
	// `DrawBox()` always draws lines with width `2`.
	// By offsetting the box by one pixel (half the line width), we avoid gaps and overlaps.
	auto offset = 1;
	canvas.SetColor(GuiColorBorder);
	canvas.SetPosition(barPos - offset);
	canvas.DrawBox(barSize + (2 * offset));

	auto values = colorRanges.GetValues();
	std::set<float> uniqueValues = std::set<float>(values.begin(), values.end());
	for (float value : uniqueValues)
	{
		if (value <= minValue || maxValue <= value) continue;

		auto start = barPos + Vector2F{ valueToPosition(value), 0 };
		auto end = start + Vector2F{ 0, barSize.Y };
		canvas.SetColor(GuiColorBorder);
		canvas.DrawLine(start, end, 2); // Line width `2` to match `DrawBox()`.
	}

	// Draw text
	canvas.SetColor(GuiColorBorder);
	canvas.SetPosition(barPos + Vector2F{ 0, barSize.Y });
	canvas.DrawString(text + toPrecision(value, 1) + " ms", FontSize(), FontSize());
}

static void DrawCenteredText(CanvasWrapper canvas, std::string text, float fontSize, Vector2F topLeft, Vector2F boxSize)
{
	Vector2F textSize = canvas.GetStringSize(text, fontSize, fontSize);
	canvas.SetPosition(topLeft + (boxSize - textSize) / 2);
	canvas.DrawString(text, fontSize, fontSize);
}

void FastAerialTrainer::DrawPitchHistory(CanvasWrapper& canvas, Vector2F position)
{
	float borderWidth = 2; // Used in `CanvasWrapper::DrawBox()`. Can't be configured...
	float textWidth = 45 * FontSize();
	Vector2F topLeft = position + Vector2F{ borderWidth, 0 };
	Vector2F innerBoxSize = Vector2F{ GuiSize, GuiSize / 10 };

	canvas.SetColor(GuiColorBorder);
	DrawCenteredText(
		canvas,
		"Pitch",
		FontSize(),
		topLeft + Vector2F{ innerBoxSize.X - textWidth, 0 },
		Vector2F{ textWidth, innerBoxSize.Y }
	);
	innerBoxSize -= Vector2F{ textWidth, 0 };

	canvas.SetColor(GuiColorBorder);
	canvas.SetPosition(topLeft - borderWidth);
	canvas.DrawBox(innerBoxSize + (2 * borderWidth));

	if (GuiShowPitchDownInHistory)
	{
		canvas.SetColor(GuiColorBorder);
		Vector2F start = topLeft - Vector2F{ borderWidth / 2.f, 0.f } + innerBoxSize * Vector2F{ 0.f, 0.5f };
		Vector2F end = start + Vector2F{ borderWidth, 0.f } + innerBoxSize * Vector2F{ 1.f, 0.f };
		canvas.DrawLine(start, end, borderWidth);
	}

	// Draw the pitch input graph.

	canvas.SetColor(GuiPitchHistoryColor);
	auto FillTriangle = [&](Vector2F p1, Vector2F p2, Vector2F p3)
		{
			if (p1.Y == p2.Y && p2.Y == p3.Y)
				return;

			transparentTriangle->Render(
				canvas,
				p1 * innerBoxSize + topLeft,
				p2 * innerBoxSize + topLeft,
				p3 * innerBoxSize + topLeft
			);
		};
	auto FillBox = [&](Vector2F p1, Vector2F p2)
		{
			canvas.SetPosition(p1 * innerBoxSize + topLeft);
			canvas.FillBox((p2 - p1) * innerBoxSize);
		};
	int historySize = (int)InputHistory.size();
	for (int i = 1; i < historySize; i++)
	{
		auto& previousInput = InputHistory[i - 1];
		auto& currentInput = InputHistory[i];

		// Which value should represent zero in the graph.
		// All x,y values are between 0 and 1, where (0,0) is top left.
		float zero;
		float startX = (float)(i - 1) / (historySize - 1);
		float endX = (float)i / (historySize - 1);
		float startY;
		float endY;

		if (GuiShowPitchDownInHistory)
		{
			zero = 0.5f;
			startY = 0.5f * (1.f - previousInput.pitch);
			endY = 0.5f * (1.f - currentInput.pitch);
		}
		else
		{
			zero = 1.f;
			startY = 1.f - std::clamp(previousInput.pitch, 0.f, 1.f);
			endY = 1.f - std::clamp(currentInput.pitch, 0.f, 1.f);
		}

		Vector2F start = Vector2F{ startX, startY };
		Vector2F end = Vector2F{ endX, endY };

		if (startY == zero && endY == zero)
		{
			// Do nothing...
		}
		else if (startY < zero && endY < zero) // Above zero
		{
			// Draw a right triangle from `start` to `end` and a rectangle beneath it.
			Vector2F base = startY > endY ? Vector2F{ endX, startY } : Vector2F{ startX, endY };

			FillTriangle(base, start, end);
			FillBox(Vector2F{ startX, std::max(startY, endY) }, Vector2F{ endX, zero });
		}
		else if (startY > zero && endY > zero) // Below zero
		{
			// Draw a right triangle from `start` to `end` and a rectangle above it.
			Vector2F base = startY < endY ? Vector2F{ endX, startY } : Vector2F{ startX, endY };

			FillTriangle(base, start, end);
			FillBox(Vector2F{ startX, zero }, Vector2F{ endX, std::min(startY, endY) });
		}
		else // Through zero
		{
			// Draw two right triangles from `start` to `end`.
			auto slope = (endY - startY) / (endX - startX);
			auto centerX = startX + (zero - startY) / slope;

			Vector2F baseLeft = Vector2F{ startX, zero };
			Vector2F baseCenter = Vector2F{ centerX , zero };
			Vector2F baseRight = Vector2F{ endX, zero };

			FillTriangle(start, baseLeft, baseCenter);
			FillTriangle(end, baseRight, baseCenter);
		}
	}

	// Highlight the jump input.

	for (int i = 0; i < historySize; i++)
	{
		auto& input = InputHistory[i];

		if (!input.jumped)
			continue;

		canvas.SetColor(GuiColorBorder);
		Vector2F start = topLeft + Vector2F{ (float)i / historySize * innerBoxSize.X, 0.f };
		Vector2F end = start + Vector2F{ 0, innerBoxSize.Y };
		canvas.DrawLine(start, end, 2 * borderWidth);
	}

	// Draw markers for every n-th tick.

	if (GuiTicksPerMarker > 0)
	{
		auto markerHeight = 0.2f;

		for (int i = GuiTicksPerMarker; i < historySize; i += GuiTicksPerMarker)
		{
			canvas.SetColor(GuiColorBorder);
			Vector2F start = topLeft + Vector2F{ (float)i / historySize * innerBoxSize.X, (1 - markerHeight) * innerBoxSize.Y };
			Vector2F end = start + Vector2F{ 0, markerHeight * innerBoxSize.Y };
			canvas.DrawLine(start, end, borderWidth);
		}
	}
}

void FastAerialTrainer::DrawBoostHistory(CanvasWrapper& canvas, Vector2F position)
{
	float borderWidth = 2;
	float textWidth = 45 * FontSize();
	Vector2F topLeft = position + Vector2F{ borderWidth, 0 };
	Vector2F innerBoxSize = BarSize();

	canvas.SetColor(GuiColorBorder);
	DrawCenteredText(
		canvas,
		"Boost",
		FontSize(),
		topLeft + Vector2F{ innerBoxSize.X - textWidth, 0 },
		Vector2F{ textWidth, innerBoxSize.Y }
	);
	innerBoxSize -= Vector2F{ textWidth, 0 };

	canvas.SetColor(GuiColorBorder);
	canvas.SetPosition(topLeft - borderWidth);
	canvas.DrawBox(innerBoxSize + (2 * borderWidth));

	int i = 0;
	int size = (int)InputHistory.size();
	for (InputHistoryItem input : InputHistory)
	{
		if (input.boost)
		{
			canvas.SetColor(GuiPitchHistoryColor);
			canvas.SetPosition(topLeft + innerBoxSize * Vector2F{ (float)i / size, 0 });
			canvas.FillBox(innerBoxSize * Vector2F{ 1.f / size, 1.f });
		}
		i++;
	}
}

void FastAerialTrainer::onUnload()
{
	// nothing to unload...
}
