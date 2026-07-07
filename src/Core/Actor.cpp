#include "Core/Actor.h"

Actor::Actor()
	: mapX(0)
	, mapY(0)
	, currentDir(Direction::DOWN)
	, currentFrame(0)
	, animTick(0)
	, isMoving(false)
{
}

void Actor::UpdateAnimation()
{
	if (isMoving)
	{
		animTick = animTick + 1;
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

void Actor::DrawSprite(BITMAP *dest, BITMAP *spriteSheet, int drawX, int drawY) const
{
	if (!spriteSheet)
		return;

	int srcX = currentFrame * CHARACTER_SPRITE_WIDTH;
	int srcY = static_cast<int>(currentDir) * CHARACTER_SPRITE_HEIGHT;
	masked_blit(spriteSheet, dest, srcX, srcY, drawX, drawY, CHARACTER_SPRITE_WIDTH, CHARACTER_SPRITE_HEIGHT);
}
