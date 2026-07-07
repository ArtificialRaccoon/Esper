#pragma once
#include <string>
#include <cstring>
#include <vector>
#include <sstream>
#include <algorithm>
#include "Core/GlobalEnumerations.h"

class StringUtils
{
	public:
		static std::vector<Direction> ParseMovePath(const std::string &pathStr)
		{
			std::vector<Direction> movePath;
			std::stringstream ss(pathStr);
			std::string token;
			while (std::getline(ss, token, ','))
			{
				token.erase(0, token.find_first_not_of(" \t\r\n"));
				token.erase(token.find_last_not_of(" \t\r\n") + 1);
				if (token.empty())
					continue;

				char c = std::toupper(token[0]);
				if (c == 'U')
					movePath.push_back(Direction::UP);
				else if (c == 'D')
					movePath.push_back(Direction::DOWN);
				else if (c == 'L')
					movePath.push_back(Direction::LEFT);
				else if (c == 'R')
					movePath.push_back(Direction::RIGHT);
			}
			return movePath;
		}
		
		static std::string SafeString(const char *data, size_t maxLen)
		{
			size_t len = 0;
			while (len < maxLen && data[len] != '\0')
			{
				len++;
			}
			return std::string(data, len);
		}

		static void TrimTrailingSpaces(char *str)
		{
			if (!str)
				return;
			size_t len = std::strlen(str);
			while (len > 0 && str[len - 1] == ' ')
			{
				len--;
				str[len] = '\0';
			}
		}
};
