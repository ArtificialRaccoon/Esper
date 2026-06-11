#pragma once
#include <string>
#include <vector>
#include <cstdint>

struct __attribute__((packed)) DatabaseHeader
{
	char magic[4];
	uint16_t version;
	uint16_t mapCount;
	uint16_t startMapIndex;
	uint16_t startTileX;
	uint16_t startTileY;
	uint16_t reserved;
};

class GameDatabase
{
	public:
		static GameDatabase &Instance()
		{
			static GameDatabase instance;
			return instance;
		}
		bool Load(const std::string &filePath);
		const DatabaseHeader &GetHeader() const { return header; }
		const std::vector<std::string> &GetMapNames() const { return mapNames; }
		std::string GetStartingMapName() const;
		uint16_t GetStartingTileX() const { return header.startTileX; }
		uint16_t GetStartingTileY() const { return header.startTileY; }

	private:
		GameDatabase() = default;
		~GameDatabase() = default;
		DatabaseHeader header{};
		std::vector<std::string> mapNames;
};