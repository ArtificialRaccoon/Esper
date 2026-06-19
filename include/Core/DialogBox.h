#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include "allegro.h"
#include "Core/CommonGUI.h"
#include "Core/GameDefines.h"

class DialogBox
{
	public:
		DialogBox() = default;
		~DialogBox() = default;
		void SetText(const std::string &rawText, int speed = 2);
		void Update();
		void Draw(BITMAP *dest, int scrollXOffset, int scrollYOffset);
		void Advance();
		void SetTextColor(int color) { textColor = color; }
		int GetTextColor() const { return textColor; }		
		bool IsActive() const { return active; }
		bool IsFinished() const { return isFinished; }
		
	private:
		std::string fullText;
		std::vector<std::string> lines;
		size_t currentCharIndex = 0;
		int textSpeed = 2;
		int paddingX = 12;
		int paddingY = 10;
		int lineHeight = 12;
		int textColor = 16;
		int marginX = 10;
		int marginY = 10;
		int boxHeight = 50;
		bool isFinished = false;
		bool active = false;
};