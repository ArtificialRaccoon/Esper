#pragma once
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <utility>
#include <memory>
#include <allegro.h>
#include "Core/GameDefines.h"
#include "Core/TileAnimation.h"
#include "Core/TileMapLayer.h"
#include "Events/Event.h"

struct __attribute__((packed)) TileMapHeader
{
	uint16_t layerCount;
	uint16_t tileAnimationCount;
	uint16_t eventCount;
};

class TileMap
{
	public:
		TileMap() = default;
		void Clear();
		void Load(const std::string &filePath);
		void LoadLayerAnimations(TileMapHeader header, std::ifstream &file);
		void LoadLayers(TileMapHeader header, std::ifstream &file);
		void LoadEvents(TileMapHeader header, std::ifstream &file);		
		void Draw(BITMAP *dest, BITMAP *tileset, int scrollTileX, int scrollTileY) const;
		void DrawUpper(BITMAP *dest, BITMAP *tileset, int scrollTileX, int scrollTileY) const;
		bool IsWalkable(int tileX, int tileY) const;
		bool CheckCollision(int playerX, int playerY, int width, int height) const;
		bool Update();
		int GetCollisionWidth() const { return collisionWidth; }
		int GetCollisionHeight() const { return collisionHeight; }
		const std::vector<std::unique_ptr<Event>>& GetEvents() const { return events; }
		std::vector<std::unique_ptr<Event>>& GetEvents() { return events; }
		const std::vector<TileMapLayer>& GetLayers() const { return layers; }
		const std::vector<TileAnimation>& GetTileAnimations() const { return tileAnimations; }		
		const std::vector<bool>& GetCollisionData() const { return collisionData; }		

	private:		
		void DrawLayer(BITMAP *dest, BITMAP *tileset, const TileMapLayer &layer, int scrollTileX, int scrollTileY) const;		
		void ReadBytes(std::istream &is, void *dest, int size);
		bool IsUpperLayer(const TileMapLayer &layer) const;
		int GetAnimatedTileId(int tileId) const;

	private:
		std::vector<bool> collisionData;
		std::vector<int> tileAnimationLookup;					
		std::vector<std::unique_ptr<Event>> events;
		std::vector<TileMapLayer> layers;
		std::vector<TileAnimation> tileAnimations;
		int collisionWidth = 0;
		int collisionHeight = 0;
};