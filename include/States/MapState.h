#pragma once
#include <stdexcept>
#include <cstdio>
#include <algorithm>
#include <unordered_map>
#include <string>
#include "GameProcessor.h"
#include "States/BaseState.h"
#include "Core/GameDefines.h"
#include "Core/CommonGUI.h"
#include "Core/GameDatabase.h"
#include "Core/Player.h"
#include "Core/TileMap.h"
#include "Core/GlobalEnumerations.h"
#include "Core/DialogBox.h"
#include "Core/ChoiceBox.h"
#include "Utilities/InputManager.h"
#include "Utilities/AudioManager.h"
#include "Events/Event.h"
#include "Events/EventInterpreter.h"
#include "Core/IGameContext.h"
#include "Core/RenderSystem.h"

class MapState : public BaseState, public IGameContext
{
	public:
		MapState() = default;
		~MapState() override { UnloadResources(); }
		void InitState(GameProcessor *game) override;
		void Pause() override { }
		void Resume() override { set_palette(CommonGUI::Instance().GetPalette()); }
		void AcquireInput(GameProcessor *game) override;
		void ProcessInput(GameProcessor *game) override;
		void FrameRender(GameProcessor *game) override;
		void UnloadResources() override;

		void ShowText(const std::string &text) override;
		void ShowChoices(const std::vector<std::string> &choices) override;
		bool IsChoiceActive() const override;
		int GetSelectedChoice() const override;
		void TransferPlayer(const std::string &mapName, int tileX, int tileY) override;
		const std::string& GetCurrentMapName() const override { return currentMapName; }

		bool IsDialogActive() const override;
		bool IsDialogTyping() const override;
		void CloseDialog() override;
		bool IsFading() const override;
		Actor* GetActorById(int id) override;
		Event* GetEventById(int id) override;
		void PlayBGM(const std::string &bgmName) override;
		void FadeOut(int speed) override;
		void FadeIn(int speed) override;
		void StartEventScript(const std::vector<std::shared_ptr<IEventCommand>> &commands, uint16_t eventId) override;

	private:
		TileMap tileMap;
		BITMAP *tileset = nullptr;
		Player player;
		int scrollX = 0;
		int scrollY = 0;
		int mapWidthPx = 0;
		int mapHeightPx = 0;
		int prevScrollTileX = -1;
		int prevScrollTileY = -1;
		bool tilemapChanged = false;
		DialogBox dialogBox;
		ChoiceBox choiceBox;
		std::string currentMapName;
		uint16_t activeEventId = 0;
		RenderSystem renderSystem;
		EventInterpreter interpreter;
		std::unordered_map<uint16_t, EventInterpreter> parallelTasks;

	private:
		void MapTransition(const std::string &mapName, int targetTileX, int targetTileY);
		bool CheckEventCollision(int targetX, int targetY);
		bool IsWalkable(int targetMapX, int targetMapY);
		void UpdateParallelTasks();
};