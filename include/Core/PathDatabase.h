#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <cstdint>

struct __attribute__((packed)) PathEntry
{
	uint16_t index;
	uint32_t offset;
};

class PathDatabase
{
	public:
		static PathDatabase &Instance()
		{
			static PathDatabase instance;
			return instance;
		}
		
		void Load(const std::string &filePath);
		std::string GetPath(uint16_t index);

	private:
		PathDatabase() = default;
		~PathDatabase();

	private:
		std::string binaryFilePath;
		std::vector<uint32_t> pathOffsets;
		std::ifstream fileStream;
};
