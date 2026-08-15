#include "Core/PathDatabase.h"

PathDatabase::~PathDatabase()
{
	if (fileStream.is_open())
		fileStream.close();
}

void PathDatabase::Load(const std::string &filePath)
{
	if (fileStream.is_open())
		fileStream.close();

	binaryFilePath = filePath;
	fileStream.open(binaryFilePath, std::ios::binary);

	uint16_t pathCount = 0;
	if (fileStream.read(reinterpret_cast<char *>(&pathCount), sizeof(pathCount)))
	{
		pathOffsets.assign(pathCount, 0);

		for (uint16_t i = 0; i < pathCount; i++)
		{
			PathEntry entry;
			fileStream.read(reinterpret_cast<char *>(&entry), sizeof(entry));
			if (entry.index < pathCount)
				pathOffsets[entry.index] = entry.offset;
		}
	}
}

std::string PathDatabase::GetPath(uint16_t index)
{
	if (index >= pathOffsets.size())
		return "";

	uint32_t offset = pathOffsets[index];
	if (offset == 0)
		return "";
	
	fileStream.seekg(offset, std::ios::beg);
	if (!fileStream)
	{
		fileStream.clear();
		fileStream.seekg(offset, std::ios::beg);
	}

	uint16_t length = 0;
	fileStream.read(reinterpret_cast<char *>(&length), sizeof(length));
	if (length == 0)
	{
		fileStream.clear();
		return "";
	}

	std::vector<char> buffer(length);
	fileStream.read(buffer.data(), length);
	return std::string(buffer.data(), length);
}
