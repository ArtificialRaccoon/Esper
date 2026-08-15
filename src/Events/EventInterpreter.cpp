#include "Events/EventInterpreter.h"
#include "Core/IGameContext.h"
#include "Core/Actor.h"
#include "Events/Event.h"

void EventInterpreter::Start(const std::vector<std::shared_ptr<IEventCommand>> &cmds, uint16_t triggeringEventId, IGameContext &context)
{
	if (cmds.empty())
	{
		isActive = false;
		return;
	}

	commands = cmds;
	currentCmdIndex = 0;
	isActive = true;
	currentWaitState = CommandResult::Continue();
	triggerId = triggeringEventId;

	Update(context);
}

void EventInterpreter::Clear()
{
	isActive = false;
	commands.clear();
	currentCmdIndex = 0;
	currentWaitState = CommandResult::Continue();
	triggerId = 0;
}

void EventInterpreter::Update(IGameContext &context)
{
	if (!isActive || context.IsDialogActive() || context.IsFading())
		return;

	if (currentWaitState.status == CommandStatus::WAIT_FRAMES)
	{
		currentWaitState.data--;
		if (currentWaitState.data > 0)
			return;

		currentWaitState = CommandResult::Continue();
	}
	else if (currentWaitState.status == CommandStatus::WAIT_FOR_ACTOR)
	{
		Actor *actor = context.GetActorById(currentWaitState.data);
		if (actor && actor->HasActiveMoveRoute())
			return;

		currentWaitState = CommandResult::Continue();
	}
	else if (currentWaitState.status == CommandStatus::YIELD)
		currentWaitState = CommandResult::Continue();

	while (currentCmdIndex < commands.size())
	{
		auto cmd = commands[currentCmdIndex];
		if (!cmd)
		{
			currentCmdIndex++;
			continue;
		}

		ExecutionContext ctx{
			context,
			context.GetEventById(triggerId),
			triggerId
		};

		CommandResult result = cmd->Execute(ctx);
		currentCmdIndex++;

		switch (result.status)
		{
			case CommandStatus::CONTINUE:
				break;

			case CommandStatus::YIELD:
				currentWaitState = result;
				return;

			case CommandStatus::WAIT_FRAMES:
				if (result.data > 0)
				{
					currentWaitState = result;
					return;
				}
				break;

			case CommandStatus::WAIT_FOR_ACTOR:
			{
				Actor *actor = context.GetActorById(result.data);
				if (actor && actor->HasActiveMoveRoute())
				{
					currentWaitState = result;
					return;
				}
				break;
			}

			case CommandStatus::TERMINATE:
				Clear();
				return;
		}

		if (context.IsDialogActive() || context.IsFading())
			return;
	}

	Clear();
}

