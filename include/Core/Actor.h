#pragma once
#include <allegro.h>
#include "Core/GlobalEnumerations.h"
#include "Core/GameDefines.h"

class Actor
{
	public:
		Actor();
		virtual ~Actor() = default;

		int GetMapX() const { return mapX; }
		int GetMapY() const { return mapY; }
		int GetTileX() const { return mapX / TILE_SIZE; }
		int GetTileY() const { return mapY / TILE_SIZE; }
		Direction GetDirection() const { return currentDir; }
		int GetCurrentFrame() const { return currentFrame; }
		bool IsMoving() const { return isMoving; }

		void SetPosition(int x, int y) { mapX = x; mapY = y; }
		void SetTilePosition(int tx, int ty) { mapX = tx * TILE_SIZE; mapY = ty * TILE_SIZE; }
		void SetDirection(Direction dir) { currentDir = dir; }
		void SetMoving(bool moving) { isMoving = moving; }

		void UpdateAnimation();
		void DrawSprite(BITMAP *dest, BITMAP *spriteSheet, int drawX, int drawY) const;

	protected:
		int mapX = 0;
		int mapY = 0;
		Direction currentDir = Direction::DOWN;
		int currentFrame = 0;
		int animTick = 0;
		bool isMoving = false;
};
