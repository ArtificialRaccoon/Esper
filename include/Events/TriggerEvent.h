#pragma once
#include "Events/Event.h"

class TriggerEvent : public Event
{
	public:
		TriggerEvent() = default;
		TriggerEvent(uint16_t id, int tileX, int tileY, int endTileX = -1, int endTileY = -1);
		~TriggerEvent() override = default;

		int GetTileX() const override { return tileX; }
		int GetTileY() const override { return tileY; }
		int GetEndTileX() const override { return endTileX; }
		int GetEndTileY() const override { return endTileY; }
		int GetMapX() const override;
		int GetMapY() const override;

		Rect GetHitbox() const override;
		bool CollidesWith(const Rect &playerRect) const override;

		void Draw(BITMAP *dest, int scrollTileX, int scrollTileY) const override;

	private:
		int tileX = 0;
		int tileY = 0;
		int endTileX = 0;
		int endTileY = 0;
};
