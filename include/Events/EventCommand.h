#pragma once
#include <string>
#include <memory>
#include <cstdint>
#include "Core/GlobalEnumerations.h"

class Event;
class IGameContext;
struct EventCommand;

enum class CommandStatus
{
	CONTINUE,
	YIELD,
	WAIT_FRAMES,
	WAIT_FOR_ACTOR,
	TERMINATE
};

struct CommandResult
{
	CommandStatus status = CommandStatus::CONTINUE;
	int data = 0;

	static CommandResult Continue() { return { CommandStatus::CONTINUE, 0 }; }
	static CommandResult Yield() { return { CommandStatus::YIELD, 0 }; }
	static CommandResult WaitFrames(int frames) { return { CommandStatus::WAIT_FRAMES, frames }; }
	static CommandResult WaitForActor(int targetId) { return { CommandStatus::WAIT_FOR_ACTOR, targetId }; }
	static CommandResult Terminate() { return { CommandStatus::TERMINATE, 0 }; }
};

struct ExecutionContext
{
	IGameContext &context;
	Event *event = nullptr;
	uint16_t triggeringEventId = 0;
};

class IEventCommand
{
	public:
		virtual ~IEventCommand() = default;
		virtual CommandResult Execute(ExecutionContext &ctx) = 0;
};

class ShowTextCommand : public IEventCommand
{
	public:
		explicit ShowTextCommand(uint16_t stringId);
		CommandResult Execute(ExecutionContext &ctx) override;
	private:
		uint16_t stringId;
};

class ControlSelfSwitchCommand : public IEventCommand
{
	public:
		ControlSelfSwitchCommand(const std::string &selfSwitch, bool value);
		CommandResult Execute(ExecutionContext &ctx) override;
	private:
		std::string selfSwitch;
		bool value;
};

class ControlSelfVarCommand : public IEventCommand
{
	public:
		ControlSelfVarCommand(const std::string &selfVar, uint8_t op, int16_t val);
		CommandResult Execute(ExecutionContext &ctx) override;
	private:
		std::string selfVar;
		uint8_t op;
		int16_t val;
};

class ControlVarCommand : public IEventCommand
{
	public:
		ControlVarCommand(const std::string &varName, uint8_t op, int16_t val);
		CommandResult Execute(ExecutionContext &ctx) override;
	private:
		std::string varName;
		uint8_t op;
		int16_t val;
};

class PlaySFXCommand : public IEventCommand
{
	public:
		explicit PlaySFXCommand(const std::string &sfxName);
		CommandResult Execute(ExecutionContext &ctx) override;
	private:
		std::string sfxName;
};

class TransferPlayerCommand : public IEventCommand
{
	public:
		TransferPlayerCommand(const std::string &mapName, int16_t tileX, int16_t tileY);
		CommandResult Execute(ExecutionContext &ctx) override;
	private:
		std::string mapName;
		int16_t tileX;
		int16_t tileY;
};

class SetMoveRouteCommand : public IEventCommand
{
	public:
		SetMoveRouteCommand(int16_t targetId, uint16_t pathId, bool bypassCollision);
		CommandResult Execute(ExecutionContext &ctx) override;
	private:
		int16_t targetId;
		uint16_t pathId;
		bool bypassCollision;
};

class WaitCommand : public IEventCommand
{
	public:
		explicit WaitCommand(int16_t frames);
		CommandResult Execute(ExecutionContext &ctx) override;
	private:
		int16_t frames;
};

class WaitForMovementCommand : public IEventCommand
{
	public:
		explicit WaitForMovementCommand(int16_t targetId);
		CommandResult Execute(ExecutionContext &ctx) override;
	private:
		int16_t targetId;
};

class FadeOutCommand : public IEventCommand
{
	public:
		explicit FadeOutCommand(int16_t speed);
		CommandResult Execute(ExecutionContext &ctx) override;
	private:
		int16_t speed;
};

class FadeInCommand : public IEventCommand
{
	public:
		explicit FadeInCommand(int16_t speed);
		CommandResult Execute(ExecutionContext &ctx) override;
	private:
		int16_t speed;
};

class PlayBGMCommand : public IEventCommand
{
	public:
		explicit PlayBGMCommand(const std::string &bgmName);
		CommandResult Execute(ExecutionContext &ctx) override;
	private:
		std::string bgmName;
};

class SetFacingCommand : public IEventCommand
{
	public:
		SetFacingCommand(int16_t targetId, Direction dir);
		CommandResult Execute(ExecutionContext &ctx) override;
	private:
		int16_t targetId;
		Direction dir;
};

class SetSpeedCommand : public IEventCommand
{
	public:
		SetSpeedCommand(int16_t targetId, int16_t speed);
		CommandResult Execute(ExecutionContext &ctx) override;
	private:
		int16_t targetId;
		int16_t speed;
};

std::shared_ptr<IEventCommand> CreateEventCommand(const EventCommand &packedCmd);

