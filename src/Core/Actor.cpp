#include "Core/Actor.h"
#include <algorithm>

Actor::Actor()
	: mapX(0)
	, mapY(0)
	, currentDir(Direction::DOWN)
	, currentFrame(0)
	, animTick(0)
	, isMoving(false)
	, targetTileX(0)
	, targetTileY(0)
	, moveSpeed(2)
	, isMovingRoute(false)
{
}

void Actor::QueueMoveRoute(const std::vector<Direction> &steps)
{
	std::queue<Direction> empty;
	std::swap(moveRouteQueue, empty);

	for (const auto &step : steps)
	{
		moveRouteQueue.push(step);
	}
	isMovingRoute = false;
}

bool Actor::HasActiveMoveRoute() const
{
	return !moveRouteQueue.empty() || isMovingRoute;
}

void Actor::UpdateMoveRouteStep()
{
	if (moveRouteQueue.empty() && !isMovingRoute)
		return;

	if (isMovingRoute)
	{
		int destX = targetTileX * TILE_SIZE;
		int destY = targetTileY * TILE_SIZE;

		if (mapX < destX)
			mapX = std::min(mapX + moveSpeed, destX);
		else if (mapX > destX)
			mapX = std::max(mapX - moveSpeed, destX);

		if (mapY < destY)
			mapY = std::min(mapY + moveSpeed, destY);
		else if (mapY > destY)
			mapY = std::max(mapY - moveSpeed, destY);

		UpdateAnimation();

		if (mapX == destX && mapY == destY)
		{
			isMoving = false;
			isMovingRoute = false;
			currentFrame = 0;
			animTick = 0;
		}
		return;
	}

	// We are not moving, but we have steps in the queue
	Direction nextDir = moveRouteQueue.front();
	currentDir = nextDir;

	int currentTileX = GetTileX();
	int currentTileY = GetTileY();
	int nextTileX = currentTileX;
	int nextTileY = currentTileY;

	switch (nextDir)
	{
		case Direction::UP:    nextTileY--; break;
		case Direction::DOWN:  nextTileY++; break;
		case Direction::LEFT:  nextTileX--; break;
		case Direction::RIGHT: nextTileX++; break;
	}

	targetTileX = nextTileX;
	targetTileY = nextTileY;
	isMoving = true;
	isMovingRoute = true;
	moveRouteQueue.pop();
}

void Actor::UpdateAnimation()
{
	if (isMoving)
	{
		animTick = animTick + 1;
		if (animTick >= ACTOR_ANIM_FRAME_TICKS)
		{
			currentFrame = (currentFrame + 1) % ACTOR_ANIM_FRAME_COUNT;
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
