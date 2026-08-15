#include "Events/ActorEvent.h"
#include "Events/EventPage.h"
#include "Core/TileMap.h"
#include "Core/Player.h"
#include "Core/TextureCache.h"
#include "Utilities/Collision.h"
#include "Utilities/StringUtils.h"
#include <algorithm>
#include <cstdlib>

ActorEvent::ActorEvent(uint16_t id, int tileX, int tileY)
	: Event(id)
	, Actor()
{
	mapX = tileX * TILE_SIZE;
	mapY = tileY * TILE_SIZE;
	targetTileX = tileX;
	targetTileY = tileY;
}

int ActorEvent::GetEndTileX() const
{
	return GetTileX();
}

int ActorEvent::GetEndTileY() const
{
	return GetTileY();
}

void ActorEvent::OnPageChanged()
{
	const EventPage* activePage = GetActivePage();
	if (activePage && activePage->GetSpriteFrame() == 0 && !activePage->GetSpriteName().empty())
	{
		moveType = activePage->GetMoveType();
		moveSpeed = activePage->GetMoveSpeed() > 0 ? activePage->GetMoveSpeed() : 1;
		moveFrequency = activePage->GetMoveFrequency() > 0 ? activePage->GetMoveFrequency() : 30;
		movePath = StringUtils::ParseMovePath(activePage->GetMovePath());
		if (!movePath.empty() && pathIndex >= movePath.size())
			pathIndex = 0;
	}
}

bool ActorEvent::CheckCollisionAt(int checkTileX, int checkTileY, const TileMap &tileMap, int playerMapX, int playerMapY, const std::vector<std::unique_ptr<Event>> &allEvents)
{
	int targetPixelX = checkTileX * TILE_SIZE;
	int targetPixelY = checkTileY * TILE_SIZE;

	if (tileMap.CheckCollision(targetPixelX, targetPixelY, TILE_SIZE, TILE_SIZE))
		return true;

	int playerHitboxLeft = playerMapX + CHARACTER_HITBOX_X_OFFSET;
	int playerHitboxRight = playerHitboxLeft + CHARACTER_HITBOX_WIDTH - 1;
	int playerHitboxTop = playerMapY + CHARACTER_HITBOX_Y_OFFSET;
	int playerHitboxBottom = playerHitboxTop + CHARACTER_HITBOX_HEIGHT - 1;

	int npcLeft = targetPixelX;
	int npcRight = targetPixelX + TILE_SIZE - 1;
	int npcTop = targetPixelY;
	int npcBottom = targetPixelY + TILE_SIZE - 1;

	if (npcLeft <= playerHitboxRight && npcRight >= playerHitboxLeft && npcTop <= playerHitboxBottom && npcBottom >= playerHitboxTop)
		return true;

	for (const auto &ev : allEvents)
	{
		if (ev->GetEventId() == eventId)
			continue;

		const EventPage *page = ev->GetActivePage();
		if (page && page->GetIsWalkable() == 0)
		{
			int evStartX = ev->GetTileX();
			int evEndX = ev->GetEndTileX();
			int evStartY = ev->GetTileY();
			int evEndY = ev->GetEndTileY();

			if (checkTileX >= evStartX && checkTileX <= evEndX && checkTileY >= evStartY && checkTileY <= evEndY)
				return true;
		}
	}

	return false;
}

void ActorEvent::Update(const TileMap &tileMap, int playerMapX, int playerMapY, const std::vector<std::unique_ptr<Event>> &allEvents, bool isDialogActive)
{
	if (isFacingPlayerOverride || isDialogActive)
	{
		currentFrame = 0;
		animTick = 0;
		return;
	}

	if (HasActiveMoveRoute())
	{
		UpdateMoveRouteStep([this, &tileMap, playerMapX, playerMapY, &allEvents](int tx, int ty) {
			return !CheckCollisionAt(tx, ty, tileMap, playerMapX, playerMapY, allEvents);
		});
		return;
	}

	if (isMoving)
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
			currentFrame = 0;
			animTick = 0;
			waitTicks = moveFrequency;

			if (moveType == EventMoveType::PATH_FOLLOW && !movePath.empty())
				pathIndex = (pathIndex + 1) % movePath.size();
		}
		return;
	}

	if (moveType == EventMoveType::FIXED)
		return;

	if (waitTicks > 0)
	{
		waitTicks--;
		return;
	}

	Direction candidateDir = currentDir;
	if (moveType == EventMoveType::RANDOM_WANDER)
		candidateDir = static_cast<Direction>(std::rand() % 4);
	else if (moveType == EventMoveType::PATH_FOLLOW)
	{
		if (movePath.empty())
			return;
		candidateDir = movePath[pathIndex];
	}

	int currentTileX = GetTileX();
	int currentTileY = GetTileY();
	int nextTileX = currentTileX;
	int nextTileY = currentTileY;

	switch (candidateDir)
	{
		case Direction::UP:    nextTileY--; break;
		case Direction::DOWN:  nextTileY++; break;
		case Direction::LEFT:  nextTileX--; break;
		case Direction::RIGHT: nextTileX++; break;
	}

	currentDir = candidateDir;

	if (!CheckCollisionAt(nextTileX, nextTileY, tileMap, playerMapX, playerMapY, allEvents))
	{
		targetTileX = nextTileX;
		targetTileY = nextTileY;
		isMoving = true;
	}
	else
		waitTicks = std::max(NPC_BLOCKED_RETRY_MIN_TICKS, moveFrequency / 2);
}

void ActorEvent::TurnToFacePlayer(int playerCenterX, int playerCenterY)
{
	if (!isFacingPlayerOverride)
	{
		savedDir = currentDir;
		isFacingPlayerOverride = true;
	}

	int npcTileX = GetTileX();
	int npcTileY = GetTileY();
	int playerTileX = playerCenterX / TILE_SIZE;
	int playerTileY = playerCenterY / TILE_SIZE;

	int dTileX = playerTileX - npcTileX;
	int dTileY = playerTileY - npcTileY;

	if (std::abs(dTileX) >= std::abs(dTileY))
	{
		if (dTileX > 0)
			currentDir = Direction::RIGHT;
		else if (dTileX < 0)
			currentDir = Direction::LEFT;
	}
	else
	{
		if (dTileY > 0)
			currentDir = Direction::DOWN;
		else if (dTileY < 0)
			currentDir = Direction::UP;
	}

	currentFrame = 0;
}

void ActorEvent::ReleasePlayerFacing()
{
	if (isFacingPlayerOverride)
	{
		currentDir = savedDir;
		isFacingPlayerOverride = false;
		if (!isMoving)
			waitTicks = 0;
	}
}

void ActorEvent::OnInteractionStart(const Player &player, IGameContext &context)
{
	int playerCenterX = player.GetMapX() + CHARACTER_HITBOX_X_OFFSET + CHARACTER_HITBOX_WIDTH / 2;
	int playerCenterY = player.GetMapY() + CHARACTER_HITBOX_Y_OFFSET + CHARACTER_HITBOX_HEIGHT / 2;
	TurnToFacePlayer(playerCenterX, playerCenterY);
}

void ActorEvent::OnInteractionEnd()
{
	ReleasePlayerFacing();
}

Rect ActorEvent::GetHitbox() const
{
	Rect rect;
	rect.left = mapX;
	rect.right = rect.left + TILE_SIZE - 1;
	rect.top = mapY;
	rect.bottom = rect.top + TILE_SIZE - 1;
	return rect;
}

bool ActorEvent::CollidesWith(const Rect &playerRect) const
{
	const EventPage* activePage = GetActivePage();
	if (!activePage || activePage->GetIsWalkable() != 0)
		return false;

	return Collision::RectOverlaps(playerRect, GetHitbox());
}

void ActorEvent::Draw(BITMAP *dest, int scrollTileX, int scrollTileY) const
{
	const EventPage* activePage = GetActivePage();
	if (!activePage || activePage->GetSpriteName().empty())
		return;

	BITMAP *spriteSheet = TextureCache::Instance().Get(activePage->GetSpriteName());
	if (spriteSheet == nullptr)
		return;

	int drawX = GetMapX() - scrollTileX * TILE_SIZE;
	int drawY = GetMapY() - scrollTileY * TILE_SIZE;

	if (drawX >= -CHARACTER_SPRITE_WIDTH && drawX < VSCREEN_W && drawY >= -CHARACTER_SPRITE_HEIGHT && drawY < VSCREEN_H)
	{
		int spriteDrawX = drawX - CHARACTER_HITBOX_X_OFFSET;
		int spriteDrawY = drawY - CHARACTER_HITBOX_Y_OFFSET;
		DrawSprite(dest, spriteSheet, spriteDrawX, spriteDrawY);
	}
}
