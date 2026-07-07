#include "Core/RenderSystem.h"
#include "Core/Player.h"
#include "Core/TileMap.h"
#include "Events/Event.h"
#include "Core/GameDefines.h"
#include <algorithm>

void RenderSystem::DrawEntities(BITMAP *dest, const Player &player, TileMap &tileMap, const std::string &currentMapName, int scrollTileX, int scrollTileY)
{
	renderList.clear();
	renderList.push_back({ player.GetSortY(), &player });

	int minX = scrollTileX * TILE_SIZE - CHARACTER_SPRITE_WIDTH;
	int maxX = (scrollTileX + VSCREEN_TILE_W) * TILE_SIZE;
	int minY = scrollTileY * TILE_SIZE - CHARACTER_SPRITE_HEIGHT;
	int maxY = (scrollTileY + VSCREEN_TILE_H) * TILE_SIZE;

	for (auto &event : tileMap.GetEvents())
	{
		const EventPage *activePage = event->GetActivePage();
		if (activePage && !activePage->GetSpriteName().empty())
		{
			int evMapX = event->GetMapX();
			int evMapY = event->GetMapY();
			if (evMapX >= minX && evMapX <= maxX && evMapY >= minY && evMapY <= maxY)
				renderList.push_back({ event->GetSortY(), event.get() });
		}
	}

	std::sort(renderList.begin(), renderList.end(), [](const RenderEntity &a, const RenderEntity &b)
	{
		return a.mapY < b.mapY;
	});

	for (const auto &entity : renderList)
	{
		entity.target->Draw(dest, scrollTileX, scrollTileY);
	}
}
