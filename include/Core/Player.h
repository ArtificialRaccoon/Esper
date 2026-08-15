#pragma once
#include <functional>
#include "Core/Actor.h"
#include "Core/IRenderable.h"

class Player : public Actor, public IRenderable
{
	public:
		Player() { moveSpeed = 4; }
		~Player() override;
		bool Load(const char *filename);
		void Update(bool isMoving, Direction dir);
		void ProcessMovementInput(int mapWidthPx, int mapHeightPx, const std::function<bool(int, int)> &isWalkable);

	public:
		int GetSortY() const override;
		void Draw(BITMAP *dest, int scrollTileX, int scrollTileY) const override;

	private:
		BITMAP *spriteSheet = nullptr;
};