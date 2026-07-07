#pragma once
#include <string>

class IGameContext
{
	public:
		virtual ~IGameContext() = default;
		virtual void ShowText(const std::string &text) = 0;
		virtual void TransferPlayer(const std::string &mapName, int tileX, int tileY) = 0;
		virtual const std::string& GetCurrentMapName() const = 0;
};
