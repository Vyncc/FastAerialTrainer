#include "pch.h"
#include "RangeList.h"

#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

#include "bakkesmod/wrappers/wrapperstructs.h"
#include "logging.h"

RangeList::RangeList(std::vector<float> values, std::vector<LinearColor*> colors)
{
	if (colors.size() != values.size() - 1)
	{
		LOG("Constructing RangeList: Number of values and colors don't match!");
		return;
	}

	this->values = values;
	this->colors = colors;
}

void RangeList::UpdateValues(std::vector<float> values)
{
	if (values.size() != this->values.size())
	{
		LOG("Updating RangeList: Number of values don't match!");
		return;
	}

	this->values = values;
}

void RangeList::UpdateValue(int index, float value)
{
	if (index < 0 || index >= values.size()) return;

	float prevValue = index - 1 < 0 ? FLT_MIN : values[index - 1];
	float nextValue = index + 1 >= values.size() ? FLT_MAX : values[index + 1];

	values[index] = std::clamp(value, prevValue, nextValue);
}

std::vector<Range> RangeList::GetRanges()
{
	std::vector<Range> ranges;

	for (int i = 0; i < colors.size(); i++)
	{
		ranges.push_back(
			{
				.min = values[i],
				.max = values[i + 1],
				.color = colors[i]
			}
		);
	}

	return ranges;
}

std::vector<float> RangeList::GetValues()
{
	return values;
}

bool RangeList::IsEmpty()
{
	return colors.empty();
}

float RangeList::GetTotalMin()
{
	if (values.empty()) return 0;
	return values.front();
}

float RangeList::GetTotalMax()
{
	if (values.empty()) return FLT_MAX;
	return values.back();
}

LinearColor* RangeList::GetColorForValue(float value)
{
	if (IsEmpty())
		return nullptr;

	for (int i = 1; i < values.size(); i++)
	{
		if (value < values[i])
			return colors[i - 1];
	}
	return colors.back();
}

std::string RangeList::ValuesToString()
{
	std::string result;

	for (int i = 0; i < values.size(); i++)
	{
		if (i > 0) result += ",";
		result += std::to_string(values[i]);
	}

	return result;
}

std::vector<float> RangeList::SplitString(std::string str)
{
	std::vector<float> values;
	std::istringstream stream(str);
	std::string value;

	while (std::getline(stream, value, ','))
	{
		values.push_back(strtof(value.c_str(), NULL));
	}

	return values;
}
