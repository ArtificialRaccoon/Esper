#pragma once
#include "Events/Event.h"
#include "Core/Actor.h"

class ActorEvent : public Event, public Actor
{
	public:
		ActorEvent() = default;
		ActorEvent(uint16_t id, int tileX, int tileY);
		~ActorEvent() override = default;

		int GetTileX() const override { return Actor::GetTileX(); }
		int GetTileY() const override { return Actor::GetTileY(); }
		int GetEndTileX() const override;
		int GetEndTileY() const override;
		int GetMapX() const override { return mapX; }
		int GetMapY() const override { return mapY; }

		void Update(const TileMap &tileMap, int playerMapX, int playerMapY, const std::vector<std::unique_ptr<Event>> &allEvents, bool isDialogActive) override;
		void OnInteractionStart(const Player &player, IGameContext &context) override;
		void OnInteractionEnd() override;
		Actor* AsActor() override { return this; }

		Rect GetHitbox() const override;
		bool CollidesWith(const Rect &playerRect) const override;

		void Draw(BITMAP *dest, int scrollTileX, int scrollTileY) const override;

	protected:
		void OnPageChanged() override;

	private:
		void TurnToFacePlayer(int playerCenterX, int playerCenterY);
		void ReleasePlayerFacing();
		bool CheckCollisionAt(int checkTileX, int checkTileY, const TileMap &tileMap, int playerMapX, int playerMapY, const std::vector<std::unique_ptr<Event>> &allEvents);

	private:
		EventMoveType moveType = EventMoveType::FIXED;
		int moveFrequency = 30;
		std::vector<Direction> movePath;

		Direction savedDir = Direction::DOWN;
		size_t pathIndex = 0;
		int waitTicks = 0;
		bool isFacingPlayerOverride = false;
};

