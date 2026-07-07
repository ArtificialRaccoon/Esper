#include "Events/TriggerEvent.h"
#include "Events/EventPage.h"
#include "Core/TextureCache.h"
#include "Utilities/Collision.h"
#include "Core/GameDefines.h"

TriggerEvent::TriggerEvent(uint16_t id, int tileX, int tileY, int endTileX, int endTileY)
	: Event(id, tileX, tileY, endTileX, endTileY)
{
}

int TriggerEvent::GetMapX() const
{
	return tileX * TILE_SIZE;
}

int TriggerEvent::GetMapY() const
{
	return tileY * TILE_SIZE;
}

Rect TriggerEvent::GetHitbox() const
{
	Rect rect;
	rect.left = tileX * TILE_SIZE;
	rect.right = (endTileX + 1) * TILE_SIZE - 1;
	rect.top = tileY * TILE_SIZE;
	rect.bottom = (endTileY + 1) * TILE_SIZE - 1;
	return rect;
}

bool TriggerEvent::CollidesWith(const Rect &playerRect) const
{
	const EventPage* activePage = GetActivePage();
	if (!activePage || activePage->GetIsWalkable() != 0)
		return false;

	return Collision::RectOverlaps(playerRect, GetHitbox());
}

void TriggerEvent::Draw(BITMAP *dest, int scrollTileX, int scrollTileY) const
{
	const EventPage* activePage = GetActivePage();
	if (!activePage || activePage->GetSpriteName().empty())
		return;

	BITMAP *spriteSheet = TextureCache::Instance().Get(activePage->GetSpriteName());
	if (spriteSheet == nullptr)
		return;

	int drawX = GetMapX() - scrollTileX * TILE_SIZE;
	int drawY = GetMapY() - scrollTileY * TILE_SIZE;

	if (activePage->GetSpriteFrame() > 0)
	{
		int frameIndex = activePage->GetSpriteFrame() - 1;
		if (drawX >= -TILE_SIZE && drawX < VSCREEN_W && drawY >= -TILE_SIZE && drawY < VSCREEN_H)
			masked_blit(spriteSheet, dest, frameIndex * TILE_SIZE, 0, drawX, drawY, TILE_SIZE, TILE_SIZE);
	}
}
