#pragma once
#include <map>
#include <queue>
#include "allegro.h"
#include "Core/GameDefines.h"

extern volatile long absTicks;
struct KeyPress
{
	int scancode;
	int ascii;
};

class InputManager
{
  	public:
		bool IsKeyDown(int keycode) const { return key[keycode] != 0; }
		bool IsKeyPressed(int keycode);
		void ResetKey(int keycode);

		void Update();
		bool GetNextKeyPress(KeyPress &outKey);

		static InputManager &Instance()
		{
			static InputManager instance;
			return instance;
		}

  	private:
		InputManager() = default;
		std::map<int, long> keyCooldown;
		std::queue<KeyPress> keyBuffer;
		const int COOLDOWN_MS = 150;
		const int REPEAT_DELAY_MS = 300;
};