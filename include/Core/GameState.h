#pragma once
#include <cstdint>
#include <vector>
#include <algorithm>

class GameState
{
	public:
		static GameState& Instance()
		{
			static GameState instance;
			return instance;
		}

	private:
		GameState() = default;
		~GameState() = default;
};