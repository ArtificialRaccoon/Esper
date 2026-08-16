#include "Core/ChoiceBox.h"

void ChoiceBox::SetChoices(const std::vector<std::string> &options)
{
	choices = options;
	selectedIndex = 0;
	active = true;
	isFinished = false;

	FONT *fontObj = CommonGUI::Instance().GetFont(0);
	int maxTextWidth = 0;
	for (const auto &choice : choices)
	{
		int textWidth = text_length(fontObj, choice.c_str());
		if (textWidth > maxTextWidth)
			maxTextWidth = textWidth;
	}

	calculatedWidth = maxTextWidth + (paddingX * 2) + cursorWidth;
	calculatedWidth = std::max(calculatedWidth, TILE_SIZE * 3);

	calculatedHeight = static_cast<int>(choices.size()) * lineHeight + (paddingY * 2);
	calculatedHeight = std::max(calculatedHeight, TILE_SIZE * 2);
}

void ChoiceBox::Reset()
{
	choices.clear();
	selectedIndex = 0;
	active = false;
	isFinished = false;
	useCustomPos = false;
}

void ChoiceBox::SelectNext()
{
	if (choices.empty())
		return;

	selectedIndex = (selectedIndex + 1) % static_cast<int>(choices.size());
}

void ChoiceBox::SelectPrevious()
{
	if (choices.empty())
		return;

	selectedIndex = (selectedIndex - 1 + static_cast<int>(choices.size())) % static_cast<int>(choices.size());
}

void ChoiceBox::Confirm()
{
	if (!active)
		return;

	active = false;
	isFinished = true;
}

void ChoiceBox::Update()
{
	if (!active)
		return;
}

void ChoiceBox::Draw(BITMAP *dest, int scrollXOffset, int scrollYOffset)
{
	if (!active || choices.empty())
		return;

	int x = 0;
	int y = 0;

	if (useCustomPos)
	{
		x = scrollXOffset + customX;
		y = scrollYOffset + customY;
	}
	else
	{
		x = scrollXOffset + SCREEN_WIDTH - marginX - calculatedWidth;
		y = scrollYOffset + SCREEN_HEIGHT - 50 - marginY - calculatedHeight - 4;
		if (y < scrollYOffset + marginY)
			y = scrollYOffset + marginY;
	}

	int width = calculatedWidth;
	int height = calculatedHeight;

	CommonGUI::Instance().DrawNineSliceBox(dest, x, y, width, height);

	FONT *fontObj = CommonGUI::Instance().GetFont(0);
	int textBaseX = x + paddingX + cursorWidth;
	int textBaseY = y + paddingY;

	for (size_t i = 0; i < choices.size(); i++)
	{
		int itemY = textBaseY + static_cast<int>(i) * lineHeight;
		if (static_cast<int>(i) == selectedIndex)
		{
			textout_ex(dest, fontObj, ">", x + paddingX, itemY, textColor, -1);
		}
		textout_ex(dest, fontObj, choices[i].c_str(), textBaseX, itemY, textColor, -1);
	}
}
