#pragma once
#include <string>
#include <cstdint>

class Player;
class TileMap;
class IGameContext;

class InteractionSystem
{
	public:
		static void ProcessAction(Player &player, TileMap &tileMap, const std::string &currentMapName, IGameContext &context, uint16_t &activeEventId);
		static void ProcessTouch(Player &player, TileMap &tileMap, const std::string &currentMapName, IGameContext &context);
};
