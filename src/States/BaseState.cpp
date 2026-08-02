#include "States/BaseState.h"
#include "GameProcessor.h"

BaseState::~BaseState() {}

void BaseState::InitState(GameProcessor *game)
{
	gameRef = game;
}

void BaseState::ChangeState(GameProcessor *game, std::unique_ptr<BaseState> newState)
{
	game->ChangeState(std::move(newState));
}