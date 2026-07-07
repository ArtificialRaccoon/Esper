#pragma once
#include <allegro.h>

class IRenderable
{
	public:
		virtual ~IRenderable() = default;
		virtual int GetSortY() const = 0;
		virtual void Draw(BITMAP *dest, int scrollTileX, int scrollTileY) const = 0;
};
