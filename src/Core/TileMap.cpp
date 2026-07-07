#include "Core/TileMap.h"
#include "Events/EventSerialization.h"
#include "Events/EventCommand.h"
#include "Utilities/StringUtils.h"
#include "Events/TriggerEvent.h"
#include "Events/ActorEvent.h"

void TileMap::Clear()
{
	layers.clear();
	tileAnimations.clear();
	tileAnimationLookup.clear();
	events.clear();
	collisionData.clear();
	collisionWidth = 0;
	collisionHeight = 0;
}

void TileMap::Load(const std::string& filePath)
{
	Clear();
	TileMapHeader header;
	std::ifstream file(filePath, std::ios::binary);	
	ReadBytes(file, &header, sizeof(header));
	LoadLayerAnimations(header, file);
	LoadLayers(header, file);
	LoadEvents(header, file);
}

void TileMap::LoadLayerAnimations(TileMapHeader header, std::ifstream& file)
{
	tileAnimations.resize(header.tileAnimationCount);
	for (auto &animation : tileAnimations)
	{
		ReadBytes(file, &animation.tileId, sizeof(animation.tileId));
		ReadBytes(file, &animation.frameCount, sizeof(animation.frameCount));
		animation.frames.resize(animation.frameCount);
		ReadBytes(file, animation.frames.data(), animation.frameCount * sizeof(TileFrame));

		animation.currentFrameIndex = 0;
		if (animation.frameCount > 0)
		{
			int durationFrames = static_cast<int>((animation.frames[0].duration * TARGET_FPS) / 1000);
			animation.framesRemaining = durationFrames > 0 ? durationFrames : 1;
		}
		else
			animation.framesRemaining = 0;
	}

	uint16_t maxTileId = 0;
	for (const auto &animation : tileAnimations)
	{
		if (animation.tileId > maxTileId)
			maxTileId = animation.tileId;
	}

	tileAnimationLookup.assign(maxTileId + 1, -1);
	for (int i = 0; i < tileAnimations.size(); i++)
	{
		tileAnimationLookup[tileAnimations[i].tileId] = static_cast<int>(i);
	}	
}

void TileMap::LoadLayers(TileMapHeader header, std::ifstream& file)
{
	for (uint16_t i = 0; i < header.layerCount; i++)
	{
		TileMapLayer layer;
		ReadBytes(file, layer.name, 8);
		layer.name[8] = '\0';
		StringUtils::TrimTrailingSpaces(layer.name);

		ReadBytes(file, &layer.width, sizeof(layer.width));
		ReadBytes(file, &layer.height, sizeof(layer.height));

		int numTiles = layer.width * layer.height;
		std::vector<uint16_t> tempTileData(numTiles);
		ReadBytes(file, tempTileData.data(), numTiles * sizeof(uint16_t));

		//I need to rethink how I am organizing the layers, this is dumb		
		if (std::strcmp(layer.name, "COL") == 0)
		{
			collisionWidth = layer.width;
			collisionHeight = layer.height;
			collisionData.resize(numTiles);
			for (int t = 0; t < numTiles; t++)
			{
				collisionData[t] = (tempTileData[t] == 0);
			}
		}
		else
		{
			layer.tileData = std::move(tempTileData);
			layers.push_back(layer);
		}
	}
}

void TileMap::LoadEvents(TileMapHeader header, std::ifstream& file)
{
	events.clear();
	for (uint16_t i = 0; i < header.eventCount; i++)
	{
		GameEvent gameEvent;
		ReadBytes(file, &gameEvent, sizeof(gameEvent));

		std::vector<EventPage> tempPages;
		bool hasSprite = false;

		for (uint16_t p = 0; p < gameEvent.pageCount; p++)
		{
			EventPageHeader pageHeader;
			ReadBytes(file, &pageHeader, sizeof(pageHeader));

			std::vector<std::shared_ptr<IEventCommand>> commands;
			for (uint16_t c = 0; c < pageHeader.commandCount; c++)
			{
				EventCommand cmd;
				ReadBytes(file, &cmd, sizeof(cmd));
				commands.push_back(CreateEventCommand(cmd));
			}

			EventPage page(pageHeader, std::move(commands));
			if (!page.GetSpriteName().empty() && page.GetSpriteFrame() == 0)
			{
				hasSprite = true;
			}
			tempPages.push_back(std::move(page));
		}

		uint16_t evId = gameEvent.eventId;
		int evTileX = gameEvent.tileX;
		int evTileY = gameEvent.tileY;
		int evEndTileX = gameEvent.endTileX;
		int evEndTileY = gameEvent.endTileY;

		std::unique_ptr<Event> ev;
		if (hasSprite)
		{
			ev = std::make_unique<ActorEvent>(
				evId,
				evTileX, evTileY
			);
		}
		else
		{
			ev = std::make_unique<TriggerEvent>(
				evId,
				evTileX, evTileY,
				evEndTileX, evEndTileY
			);
		}

		for (auto &page : tempPages)
		{
			ev->AddPage(std::move(page));
		}

		events.push_back(std::move(ev));
	}
}

bool TileMap::Update()
{
	bool changed = false;
	for (TileAnimation &animation : tileAnimations)
	{
		animation.framesRemaining--;
		if (animation.framesRemaining <= 0)
		{
			animation.currentFrameIndex = (animation.currentFrameIndex + 1) % animation.frameCount;
			int durationFrames = static_cast<int>((animation.frames[animation.currentFrameIndex].duration * TARGET_FPS) / 1000);
			animation.framesRemaining = durationFrames > 0 ? durationFrames : 1;
			changed = true;
		}
	}
	return changed;
}

void TileMap::Draw(BITMAP *dest, BITMAP *tileset, int scrollTileX, int scrollTileY) const
{
	if (layers.empty())
		return;

	clear_to_color(dest, 0);
	DrawLayer(dest, tileset, layers[0], scrollTileX, scrollTileY);
	for (int l = 1; l < layers.size(); l++)
	{
		if (!IsUpperLayer(layers[l]))
			DrawLayer(dest, tileset, layers[l], scrollTileX, scrollTileY);
	}
}

void TileMap::DrawUpper(BITMAP *dest, BITMAP *tileset, int scrollTileX, int scrollTileY) const
{
	for (int layerIndex = 1; layerIndex < layers.size(); layerIndex++)
	{
		if (IsUpperLayer(layers[layerIndex]))
			DrawLayer(dest, tileset, layers[layerIndex], scrollTileX, scrollTileY);
	}
}

bool TileMap::IsWalkable(int tileX, int tileY) const
{
	if (collisionData.empty())
		return true;
	if (tileX < 0 || tileX >= collisionWidth || tileY < 0 || tileY >= collisionHeight)
		return false;
	return collisionData[tileY * collisionWidth + tileX];
}

bool TileMap::CheckCollision(int playerX, int playerY, int width, int height) const
{
	if (playerX < 0 || playerY < 0)
		return true;

	int startTileX = playerX / TILE_SIZE;
	int endTileX = (playerX + width - 1) / TILE_SIZE;
	int startTileY = playerY / TILE_SIZE;
	int endTileY = (playerY + height - 1) / TILE_SIZE;

	for (int tileY = startTileY; tileY <= endTileY; tileY++)
	{
		for (int tileX = startTileX; tileX <= endTileX; tileX++)
		{
			if (!IsWalkable(tileX, tileY))
				return true;
		}
	}
	return false;
}

bool TileMap::IsUpperLayer(const TileMapLayer &layer) const
{
	//Again, I need to rethink how I am organizing layers.  Good enough for now.
	return (std::strncmp(layer.name, "OL", 2) == 0 || std::strncmp(layer.name, "ol", 2) == 0);
}

int TileMap::GetAnimatedTileId(int tileId) const
{
	if (tileId < tileAnimationLookup.size())
	{
		int animationIndex = tileAnimationLookup[tileId];
		if (animationIndex != -1)
		{
			const auto &animation = tileAnimations[animationIndex];
			if (animation.currentFrameIndex < animation.frameCount)
				return animation.frames[animation.currentFrameIndex].tileId;
		}
	}
	return tileId;
}

void TileMap::DrawLayer(BITMAP *dest, BITMAP *tileset, const TileMapLayer &layer, int scrollTileX, int scrollTileY) const
{
	const int tileSize = TILE_SIZE;
	const int tilesetCols = tileset->w / tileSize;

	for (int tileY = 0; tileY < VSCREEN_TILE_H; tileY++)
	{
		int mapY = scrollTileY + tileY;
		int destY = tileY * tileSize;		

		for (int tileX = 0; tileX < VSCREEN_TILE_W; tileX++)
		{
			int mapX = scrollTileX + tileX;
			int destX = tileX * tileSize;

			if ((mapY >= 0 && mapY < layer.height) && (mapX >= 0 && mapX < layer.width))
			{	
				uint16_t displayIndex = GetAnimatedTileId(layer.tileData[mapY * layer.width + mapX]);
				if (displayIndex > 0)
				{
					int tileIndex = displayIndex - 1;
					int srcX = (tileIndex % tilesetCols) * tileSize;
					int srcY = (tileIndex / tilesetCols) * tileSize;
					masked_blit(tileset, dest, srcX, srcY, destX, destY, tileSize, tileSize);
				}
			}
		}
	}
}

void TileMap::ReadBytes(std::istream &is, void *dest, int size)
{
	if (size > 0)
		is.read(reinterpret_cast<char*>(dest), size);
}
