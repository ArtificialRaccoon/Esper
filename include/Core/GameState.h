#pragma once
#include <string>
#include <unordered_map>
#include <cstdint>

class GameState
{
	public:
		static GameState& Instance()
		{
			static GameState instance;
			return instance;
		}

		bool GetSwitch(const std::string &key) const;
		void SetSwitch(const std::string &key, bool value);
		bool GetSelfSwitch(const std::string &mapName, uint16_t eventId, const std::string &switchName) const;
		void SetSelfSwitch(const std::string &mapName, uint16_t eventId, const std::string &switchName, bool value);
		int GetVariable(const std::string &key) const;
		void SetVariable(const std::string &key, int value);

	private:
		GameState() = default;
		~GameState() = default;
		std::unordered_map<std::string, bool> switches;
		std::unordered_map<std::string, bool> selfSwitches;
		std::unordered_map<std::string, int> variables;
};