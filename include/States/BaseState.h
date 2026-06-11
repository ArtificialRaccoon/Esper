#pragma once

#include <memory>
#include "allegro.h"

class GameProcessor;
class BaseState
{
	public:
		virtual ~BaseState();
		virtual void InitState();
		virtual void Pause() = 0;
		virtual void Resume() = 0;
		virtual void AcquireInput(GameProcessor *game) = 0;
		virtual void ProcessInput(GameProcessor *game) = 0;
		virtual void Render(GameProcessor *game) { FrameRender(game); }
		virtual void FrameRender(GameProcessor *game) = 0;
		virtual void UnloadResources() = 0;

	public:
		void ChangeState(GameProcessor *game, std::unique_ptr<BaseState> newState);

	protected:
		BaseState() {}
		BITMAP *BUFFER = nullptr;
		bool interactPressed = false;
};