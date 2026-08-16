#pragma once
#include <string>
#include <vector>
#include <memory>

class Actor;
class Event;
class IEventCommand;

class IGameContext
{
	public:
		virtual ~IGameContext() = default;
		virtual void ShowText(const std::string &text) = 0;
		virtual void TransferPlayer(const std::string &mapName, int tileX, int tileY) = 0;
		virtual const std::string& GetCurrentMapName() const = 0;
		
		// New interfaces for cutscene support:
		virtual bool IsDialogActive() const = 0;
		virtual bool IsDialogTyping() const = 0;
		virtual void CloseDialog() = 0;
		virtual bool IsChoiceActive() const = 0;
		virtual void ShowChoices(const std::vector<std::string> &choices) = 0;
		virtual int GetSelectedChoice() const = 0;
		virtual bool IsFading() const = 0;
		virtual Actor* GetActorById(int id) = 0;
		virtual Event* GetEventById(int id) = 0;
		virtual void PlayBGM(const std::string &bgmName) = 0;
		virtual void FadeOut(int speed) = 0;
		virtual void FadeIn(int speed) = 0;
		virtual void StartEventScript(const std::vector<std::shared_ptr<IEventCommand>> &commands, uint16_t eventId) = 0;
};
