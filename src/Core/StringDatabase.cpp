#include "Core/StringDatabase.h"

StringDatabase::~StringDatabase()
{
	if (fileStream.is_open())
		fileStream.close();
}

void StringDatabase::Load(const std::string &filePath)
{
	if (fileStream.is_open())
		fileStream.close();

	binaryFilePath = filePath;
	fileStream.open(binaryFilePath, std::ios::binary);

	uint16_t stringCount = 0;
	fileStream.read(reinterpret_cast<char *>(&stringCount), sizeof(stringCount));
	stringOffsets.assign(stringCount, 0);

	for (uint16_t i = 0; i < stringCount; i++)
	{
		StringEntry entry;
		fileStream.read(reinterpret_cast<char *>(&entry), sizeof(entry));
		if (entry.index < stringCount)
			stringOffsets[entry.index] = entry.offset;
	}
}

std::string StringDatabase::GetString(uint16_t index)
{
	if (index >= stringOffsets.size())
		return "";

	uint32_t offset = stringOffsets[index];
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
