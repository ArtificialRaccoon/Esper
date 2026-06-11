#include "Core/GameDatabase.h"
#include <fstream>
#include <stdexcept>
#include <cstring>

bool GameDatabase::Load(const std::string &filePath)
{
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open())
		return false;

	if (!file.read(reinterpret_cast<char *>(&header), sizeof(header)))
		return false;

	if (header.magic[0] != 'B' || header.magic[1] != 'E' || header.magic[2] != 'A' || header.magic[3] != 'R')
		return false;

	mapNames.clear();
	mapNames.reserve(header.mapCount);

	for (uint16_t i = 0; i < header.mapCount; ++i)
	{
		char filename[32];
		if (!file.read(filename, sizeof(filename)))
			return false;

		filename[31] = '\0';
		mapNames.push_back(std::string(filename));
	}

	return true;
}

std::string GameDatabase::GetStartingMapName() const
{
	if (header.startMapIndex < mapNames.size())
		return mapNames[header.startMapIndex];
	return "";
}
