#pragma once
#include "allegro.h"
#include "GameDefines.h"
#include "GlobalEnumerations.h"

class Player
{
	public:
		Player();
		~Player();

		bool Load(const char *filename);
		void Update(bool isMoving, Direction dir);
		void Draw(BITMAP *dest, int screenX, int screenY);

		Direction GetDirection() const { return currentDir; }
		int GetCurrentFrame() const { return currentFrame; }
		bool IsMoving() const { return moving; }

	private:
		BITMAP *spriteSheet = nullptr;
		Direction currentDir = Direction::DOWN;
		int currentFrame = 0;
		int animTick = 0;
		bool moving = false;
};