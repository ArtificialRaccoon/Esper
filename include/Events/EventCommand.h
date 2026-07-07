#pragma once
#include <string>
#include <memory>

class Event;
class IGameContext;
struct EventCommand;

class IEventCommand
{
	public:
		virtual ~IEventCommand() = default;
		virtual bool Execute(Event &event, IGameContext &context) = 0;
};

class ShowTextCommand : public IEventCommand
{
	public:
		explicit ShowTextCommand(uint16_t stringId);
		bool Execute(Event &event, IGameContext &context) override;
	private:
		uint16_t stringId;
};

class ControlSelfSwitchCommand : public IEventCommand
{
	public:
		ControlSelfSwitchCommand(const std::string &selfSwitch, bool value);
		bool Execute(Event &event, IGameContext &context) override;
	private:
		std::string selfSwitch;
		bool value;
};

class ControlSelfVarCommand : public IEventCommand
{
	public:
		ControlSelfVarCommand(const std::string &selfVar, uint8_t op, int16_t val);
		bool Execute(Event &event, IGameContext &context) override;
	private:
		std::string selfVar;
		uint8_t op;
		int16_t val;
};

class ControlVarCommand : public IEventCommand
{
	public:
		ControlVarCommand(const std::string &varName, uint8_t op, int16_t val);
		bool Execute(Event &event, IGameContext &context) override;
	private:
		std::string varName;
		uint8_t op;
		int16_t val;
};

class PlaySFXCommand : public IEventCommand
{
	public:
		explicit PlaySFXCommand(const std::string &sfxName);
		bool Execute(Event &event, IGameContext &context) override;
	private:
		std::string sfxName;
};

class TransferPlayerCommand : public IEventCommand
{
	public:
		TransferPlayerCommand(const std::string &mapName, int16_t tileX, int16_t tileY);
		bool Execute(Event &event, IGameContext &context) override;
	private:
		std::string mapName;
		int16_t tileX;
		int16_t tileY;
};

std::shared_ptr<IEventCommand> CreateEventCommand(const EventCommand &packedCmd);
