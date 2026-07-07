#pragma once
#include <algorithm>

struct Rect
{
	int left = 0;
	int right = 0;
	int top = 0;
	int bottom = 0;

	Rect() = default;
	Rect(int l, int r, int t, int b) : left(l), right(r), top(t), bottom(b) {}
};

class Collision
{
	public:
		static bool RectOverlaps(const Rect &r1, const Rect &r2)
		{
			return r1.left <= r2.right && r1.right >= r2.left && r1.top <= r2.bottom && r1.bottom >= r2.top;
		}

		static int GetOverlapArea(const Rect &r1, const Rect &r2)
		{
			return std::max(0, std::min(r1.right, r2.right) - std::max(r1.left, r2.left) + 1) *
			       std::max(0, std::min(r1.bottom, r2.bottom) - std::max(r1.top, r2.top) + 1);
		}
};
