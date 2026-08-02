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

class BaseState;
class GameProcessor
{
  	public:
		void InitializeGame();
		void HandleEvents() { currentState->ProcessInput(this); }
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
		void SetScrollOffset(int x, int y) { scrollX = x; scrollY = y; }

  	private:
		bool isRunning = true;
		bool shouldFadeIn = false;		
		int activePage = 0;
		int scrollX = 0;
		int scrollY = 0;
		int fadeDirection = 1;
		float deltaFade = 0;	
		BITMAP *videoPages[2] = {nullptr, nullptr};	
		std::unique_ptr<BaseState> currentState;
		std::vector<std::unique_ptr<BaseState>> stateStack;
		std::unordered_map<std::string, std::function<std::unique_ptr<BaseState>()>> stateRegistry;
};