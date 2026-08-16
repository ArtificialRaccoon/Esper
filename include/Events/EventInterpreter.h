#pragma once
#include <vector>
#include <memory>
#include "Events/EventCommand.h"

class IGameContext;

struct ExecutionFrame
{
	std::vector<std::shared_ptr<IEventCommand>> commands;
	size_t currentCmdIndex = 0;
};

class EventInterpreter
{
	public:
		EventInterpreter() = default;
		~EventInterpreter() = default;

		void Start(const std::vector<std::shared_ptr<IEventCommand>> &cmds, uint16_t triggeringEventId, IGameContext &context, bool loop = false);
		void Update(IGameContext &context);
		bool IsActive() const { return isActive; }
		bool IsLooping() const { return isLooping; }
		uint16_t GetTriggerId() const { return triggerId; }
		void Clear();
		void Stop() { Clear(); }

	private:
		bool isActive = false;
		bool isLooping = false;
		std::vector<std::shared_ptr<IEventCommand>> rootCommands;
		std::vector<ExecutionFrame> frameStack;
		CommandResult currentWaitState = CommandResult::Continue();
		uint16_t triggerId = 0;
};
