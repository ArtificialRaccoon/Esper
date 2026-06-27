#include "Core/GameState.h"

bool GameState::GetSwitch(const std::string &key) const
{
	auto it = switches.find(key);
	if (it != switches.end())
		return it->second;
	return false;
}

void GameState::SetSwitch(const std::string &key, bool value)
{
	switches[key] = value;
}

bool GameState::GetSelfSwitch(const std::string &mapName, uint16_t eventId, const std::string &switchName) const
{
	std::string key = mapName + "_" + std::to_string(eventId) + "_" + switchName;
	auto it = selfSwitches.find(key);
	if (it != selfSwitches.end())
		return it->second;
	return false;
}

void GameState::SetSelfSwitch(const std::string &mapName, uint16_t eventId, const std::string &switchName, bool value)
{
	std::string key = mapName + "_" + std::to_string(eventId) + "_" + switchName;
	selfSwitches[key] = value;
}

int GameState::GetVariable(const std::string &key) const
{
	auto it = variables.find(key);
	if (it != variables.end())
		return it->second;
	return 0;
}

void GameState::SetVariable(const std::string &key, int value)
{
	variables[key] = value;
}