#include <algorithm>
#include "Core/Player.h"
#include "Utilities/InputManager.h"

Player::~Player()
{
	if (spriteSheet)
	{
		destroy_bitmap(spriteSheet);
		spriteSheet = nullptr;
	}
}

bool Player::Load(const char *filename)
{
	if (spriteSheet)
	{
		destroy_bitmap(spriteSheet);
		spriteSheet = nullptr;
	}

	spriteSheet = load_bitmap(filename, nullptr);
	return spriteSheet != nullptr;
}

void Player::Update(bool isMovingVal, Direction dir)
{
	isMoving = isMovingVal;
	currentDir = dir;
	UpdateAnimation();
}

void Player::ProcessMovementInput(int mapWidthPx, int mapHeightPx, const std::function<bool(int, int)> &isWalkable)
{
	if (HasActiveMoveRoute())
		return;

	isMoving = false;
	int dx = 0;
	int dy = 0;
	int speed = moveSpeed;

	if (InputManager::Instance().IsKeyDown(KEY_UP))
	{
		dy = -speed;
		currentDir = Direction::UP;
	}
	else if (InputManager::Instance().IsKeyDown(KEY_DOWN))
	{
		dy = speed;
		currentDir = Direction::DOWN;
	}

	if (InputManager::Instance().IsKeyDown(KEY_LEFT))
	{
		dx = -speed;
		currentDir = Direction::LEFT;
	}
	else if (InputManager::Instance().IsKeyDown(KEY_RIGHT))
	{
		dx = speed;
		currentDir = Direction::RIGHT;
	}

	if (dx != 0)
	{
		int newX = std::clamp(mapX + dx, 0, mapWidthPx - CHARACTER_SPRITE_WIDTH);
		if (isWalkable(newX, mapY))
		{
			mapX = newX;
			isMoving = true;
		}
	}

	if (dy != 0)
	{
		int newY = std::clamp(mapY + dy, 0, mapHeightPx - CHARACTER_SPRITE_HEIGHT);
		if (isWalkable(mapX, newY))
		{
			mapY = newY;
			isMoving = true;
		}
	}
}

int Player::GetSortY() const
{
	return mapY + CHARACTER_HITBOX_Y_OFFSET;
}

void Player::Draw(BITMAP *dest, int scrollTileX, int scrollTileY) const
{
	int screenX = mapX - scrollTileX * TILE_SIZE;
	int screenY = mapY - scrollTileY * TILE_SIZE;
	DrawSprite(dest, spriteSheet, screenX, screenY);
}
