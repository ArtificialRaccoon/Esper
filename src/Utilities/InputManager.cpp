#include "Utilities/InputManager.h"

void InputManager::Update()
{
	while (keypressed())
	{
		int k = readkey();
		KeyPress kp;
		kp.scancode = k >> 8;
		kp.ascii = k & 0xff;
		keyBuffer.push(kp);
	}
}

bool InputManager::GetNextKeyPress(KeyPress &outKey)
{
	if (!keyBuffer.empty())
	{
		outKey = keyBuffer.front();
		keyBuffer.pop();
		return true;
	}
	return false;
}

bool InputManager::IsKeyPressed(int keycode)
{
	if (key[keycode])
	{
		long currentTime = absTicks * (1000 / TARGET_FPS);

		if (keyCooldown[keycode] == 0)
		{
			keyCooldown[keycode] = currentTime + REPEAT_DELAY_MS;
			return true;
		}
		else if (currentTime > keyCooldown[keycode])
		{
			keyCooldown[keycode] = currentTime + COOLDOWN_MS;
			return true;
		}
	}
	else
	{
		auto it = keyCooldown.find(keycode);
		if (it != keyCooldown.end())
			keyCooldown.erase(it);
	}
	
	return false;
}

void InputManager::ResetKey(int keycode)
{
	long currentTime = absTicks * (1000 / TARGET_FPS);
	keyCooldown[keycode] = currentTime + REPEAT_DELAY_MS;
}