#pragma once

#include <vector>
#include <string>

struct LinearColor;

struct Range
{
	float min;
	float max;
	LinearColor* color;
};

class RangeList
{
private:
	std::vector<float> values;
	std::vector<LinearColor*> colors;

public:
	RangeList(std::vector<float> values, std::vector<LinearColor*> colors);

	void UpdateValues(std::vector<float> values);
	void UpdateValue(int index, float value);
	std::vector<Range> GetRanges();
	std::vector<float> GetValues();

	bool IsEmpty();
	float GetTotalMin();
	float GetTotalMax();
	LinearColor* GetColorForValue(float value);

	std::string ValuesToString();
	static std::vector<float> SplitString(std::string str);
};
