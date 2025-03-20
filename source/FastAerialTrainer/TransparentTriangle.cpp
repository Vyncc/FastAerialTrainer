#include "pch.h"
#include "TransparentTriangle.h"

#include <cfloat>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <utility>

#include "bakkesmod/wrappers/wrapperstructs.h"
#include "bakkesmod/wrappers/canvaswrapper.h"
#include "bakkesmod/wrappers/ImageWrapper.h"

namespace VectorUtils
{
	float distance(Vector2F v1, Vector2F v2);
	float dot(Vector2F v1, Vector2F v2);
	float determinant(Vector2F v1, Vector2F v2);
	bool equals(Vector2F v1, Vector2F v2);
	Vector2F normalize(Vector2F v);
	Vector2F rotateLeft(Vector2F v);
	Vector2F intersect(Vector2F v1, Vector2F v2, Vector2F w1, Vector2F w2);
	Vector2F rotationCenter(Vector2F vOld, Vector2F vNew, Vector2F wOld, Vector2F wNew);
}
using namespace VectorUtils;

TransparentTriangle::TransparentTriangle(std::shared_ptr<ImageWrapper> triangleImage)
	: triangleImage(triangleImage)
{
	if (triangleImage->IsLoadedForCanvas())
		imageLoaded = true;
	else
		imageLoaded = triangleImage->LoadForCanvas();
}

void TransparentTriangle::Render(CanvasWrapper canvas, Vector2F p1, Vector2F p2, Vector2F p3)
{
	// Render an opaque triangle, if the texture failed to load.
	if (!imageLoaded)
	{
		canvas.FillTriangle(p1, p2, p3);
		return;
	}

	// Determine `c` to be opposite of the longest side.
	float d1 = distance(p2, p3);
	float d2 = distance(p3, p1);
	float d3 = distance(p1, p2);
	float longest = std::max({ d1, d2, d3 });
	Vector2F a, b, c;
	if (d1 == longest)
		c = p1, a = p2, b = p3;
	else if (d2 == longest)
		c = p2, a = p3, b = p1;
	else
		c = p3, a = p1, b = p2;

	// Make sure points are ordered counter-clockwise.
	if (determinant(a - c, b - c) > 0)
		std::swap(a, b);

	// A right triangle can be rendered immediately.
	if (std::abs(dot(normalize(a - c), normalize(b - c))) < FLT_EPSILON)
	{
		RenderRightTriangle(canvas, a, b, c);
		return;
	}

	// Find base of the height opposite to `c`.
	float t = dot(b - c, b - a) / dot(b - a, b - a);
	Vector2F d = a * t + b * (1 - t);

	// Draw two right triangles `adc` and `cdb`.
	RenderRightTriangle(canvas, c, a, d);
	RenderRightTriangle(canvas, b, c, d);
}

void TransparentTriangle::RenderRightTriangle(CanvasWrapper canvas, Vector2F a, Vector2F b, Vector2F c)
{
	// Offset one pixel from the actual image size to avoid artifacts.
	Vector2F tileStart = { 1, 1 };
	Vector2F tileSize = triangleImage->GetSizeF() - 2;

	Vector2F size = { distance(a, c), distance(b, c) };
	float angle = atan2((a - c).Y, (a - c).X);
	Rotator rotator = Rotator(0, angle * CONST_RadToUnrRot, 0);
	Vector2F center;
	if (a.Y == c.Y) // no rotation
	{
		canvas.SetPosition(b);
		center = Vector2F{ 0, 0 };
	}
	else
	{
		// Try to minimize the translation to avoid weird artifacts and inconsistencies.
		Vector2F topLeft = b;
		Vector2F overflow = topLeft + size - canvas.GetSize();
		if (overflow.X > 0) topLeft.X -= overflow.X;
		if (overflow.Y > 0) topLeft.Y -= overflow.Y;
		Vector2F bottomRight = topLeft + size;
		canvas.SetPosition(topLeft);
		center = (rotationCenter(topLeft, b, bottomRight, a) - topLeft) / size;
	}
	canvas.DrawRotatedTile(
		triangleImage.get(),
		rotator,
		size.X, size.Y,
		tileStart.X, tileStart.Y,
		tileSize.X, tileSize.Y,
		center.X, center.Y
	);
}

namespace VectorUtils
{
	float distance(Vector2F v1, Vector2F v2)
	{
		auto d = v1 - v2;
		return std::sqrtf(d.X * d.X + d.Y * d.Y);
	}

	float dot(Vector2F v1, Vector2F v2)
	{
		return v1.X * v2.X + v1.Y * v2.Y;
	}

	float determinant(Vector2F v1, Vector2F v2)
	{
		return v1.X * v2.Y - v1.Y * v2.X;
	}

	bool equals(Vector2F v1, Vector2F v2)
	{
		return v1.X == v2.X && v1.Y == v2.Y;
	}

	Vector2F normalize(Vector2F v)
	{
		float magnitude = distance(Vector2F(), v);
		return v / magnitude;
	}

	Vector2F rotateLeft(Vector2F v)
	{
		return Vector2F{ -v.Y, v.X };
	}

	Vector2F intersect(Vector2F v1, Vector2F v2, Vector2F w1, Vector2F w2)
	{
		double x1 = v1.X, x2 = v2.X, x3 = w1.X, x4 = w2.X;
		double y1 = v1.Y, y2 = v2.Y, y3 = w1.Y, y4 = w2.Y;

		// https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection#Given_two_points_on_each_line
		double denominator = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
		if (denominator == 0) return Vector2F();
		double x = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / denominator;
		double y = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / denominator;
		return Vector2F(x, y);
	}

	Vector2F rotationCenter(Vector2F vOld, Vector2F vNew, Vector2F wOld, Vector2F wNew)
	{
		if (equals(vOld, vNew)) return vOld;
		if (equals(wOld, wNew)) return wOld;

		auto vMid = (vOld + vNew) / 2;
		auto wMid = (wOld + wNew) / 2;
		auto vNormal = rotateLeft(vNew - vOld);
		auto wNormal = rotateLeft(wNew - wOld);

		return intersect(vMid, vMid + vNormal, wMid, wMid + wNormal);
	}
}
