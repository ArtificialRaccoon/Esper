#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <cstdint>

struct __attribute__((packed)) StringEntry
{
	uint16_t index;
	uint32_t offset;
};

class StringDatabase
{
	public:
		static StringDatabase &Instance()
		{
			static StringDatabase instance;
			return instance;
		}
		
		void Load(const std::string &filePath);
		std::string GetString(uint16_t index);

	private:
		StringDatabase() = default;
		~StringDatabase();

	private:
		std::string binaryFilePath;
		std::vector<uint32_t> stringOffsets;
		std::ifstream fileStream;
};
