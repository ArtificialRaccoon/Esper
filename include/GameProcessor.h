#pragma once
#include <cstdio>
#include <cstdlib>
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>
#include "allegro.h"
#include "Core/GameDefines.h"
#include "Core/GameDatabase.h"
#include "Core/StringDatabase.h"
#include "States/BaseState.h"
#include "Utilities/InputManager.h"

enum class FadeDirection
{
	NONE,
	FADE_OUT,
	FADE_IN
};

class BaseState;
class GameProcessor
{
	public:
		void InitializeGame();
		void HandleEvents();
		void ProcessEvents();
		void Update() {}
		void Render();
		void DeallocateResources();
		bool IsRunning() const { return isRunning; }
		void Quit() { isRunning = false; }

		void PopState();
		void ChangeState(std::unique_ptr<BaseState> newState);
		void PushState(std::unique_ptr<BaseState> newState);
		void RegisterState(const std::string &name, std::function<std::unique_ptr<BaseState>()> factory) { stateRegistry[name] = factory; }
		std::unique_ptr<BaseState> CreateStateByName(const std::string &name);

		BITMAP *GetBackBuffer() { return videoPages[activePage ^ 1]; }
		void FlipPages();

		void FadeOut(int speed);
		void FadeIn(int speed);
		bool IsFading() const { return isFading; }
		void UpdateFade();
		void SetScrollOffset(int x, int y) { scrollX = x; scrollY = y; }

	private:
		bool isRunning = true;
		bool isFading = false;
		int fadeStep = 0;
		int fadeSpeed = DEFAULT_FADE_SPEED;
		FadeDirection fadeDirection = FadeDirection::NONE;
		PALETTE sourcePal;
		PALETTE targetPal;
		int activePage = 0;
		int scrollX = 0;
		int scrollY = 0;
		BITMAP *videoPages[2] = {nullptr, nullptr};
		std::unique_ptr<BaseState> currentState;
		std::vector<std::unique_ptr<BaseState>> stateStack;
		std::unordered_map<std::string, std::function<std::unique_ptr<BaseState>()>> stateRegistry;
};