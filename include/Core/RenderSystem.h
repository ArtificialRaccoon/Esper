#pragma once
#include <vector>
#include <string>
#include <allegro.h>
#include "Core/IRenderable.h"

class Player;
class TileMap;

struct RenderEntity
{
	int mapY;
	const IRenderable *target;
};

class RenderSystem
{
	public:
		RenderSystem() = default;
		~RenderSystem() = default;
		void DrawEntities(BITMAP *dest, const Player &player, TileMap &tileMap, const std::string &currentMapName, int scrollTileX, int scrollTileY);

	private:
		std::vector<RenderEntity> renderList;
};
