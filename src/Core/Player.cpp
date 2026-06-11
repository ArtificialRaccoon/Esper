#include "Core/Player.h"

Player::Player() {}

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

void Player::Update(bool isMoving, Direction dir)
{
	moving = isMoving;
	currentDir = dir;

	if (moving)
	{
		animTick++;
		if (animTick >= 6)
		{
			currentFrame = (currentFrame + 1) % 4;
			animTick = 0;
		}
	}
	else
	{
		currentFrame = 0;
		animTick = 0;
	}
}

void Player::Draw(BITMAP *dest, int screenX, int screenY)
{
	if (!spriteSheet)
		return;

	int srcX = currentFrame * CHARACTER_SPRITE_WIDTH;
	int srcY = static_cast<int>(currentDir) * CHARACTER_SPRITE_HEIGHT;

	masked_blit(spriteSheet, dest, srcX, srcY, screenX, screenY, CHARACTER_SPRITE_WIDTH, CHARACTER_SPRITE_HEIGHT);
}
