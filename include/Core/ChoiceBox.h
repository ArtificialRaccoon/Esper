#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include "allegro.h"
#include "Core/CommonGUI.h"
#include "Core/GameDefines.h"

class ChoiceBox
{
	public:
		ChoiceBox() = default;
		~ChoiceBox() = default;

		void SetChoices(const std::vector<std::string> &options);
		void Update();
		void Draw(BITMAP *dest, int scrollXOffset, int scrollYOffset);

		void SelectNext();
		void SelectPrevious();
		void Confirm();

		void Reset();
		bool IsActive() const { return active; }
		bool IsFinished() const { return isFinished; }
		int GetSelectedIndex() const { return selectedIndex; }

		void SetTextColor(int color) { textColor = color; }
		int GetTextColor() const { return textColor; }

		void SetCustomPosition(int x, int y)
		{
			useCustomPos = true;
			customX = x;
			customY = y;
		}

		void ClearCustomPosition()
		{
			useCustomPos = false;
		}

	private:
		std::vector<std::string> choices;
		int selectedIndex = 0;
		bool active = false;
		bool isFinished = false;

		bool useCustomPos = false;
		int customX = 0;
		int customY = 0;

		int paddingX = 10;
		int paddingY = 8;
		int lineHeight = 12;
		int cursorWidth = 10;
		int textColor = 16;
		int marginX = 10;
		int marginY = 10;

		int calculatedWidth = 0;
		int calculatedHeight = 0;
};
