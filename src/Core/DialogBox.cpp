#include "Core/DialogBox.h"

void DialogBox::SetText(const std::string &rawText, int speed)
{
	std::string processedText = "";
	for (size_t i = 0; i < rawText.length(); i++)
	{
		if (rawText[i] == '\\' && i + 1 < rawText.length() && rawText[i + 1] == 'n')
		{
			processedText += '\n';
			i++;
		}
		else
			processedText += rawText[i];
	}

	fullText = processedText;
	lines.clear();
	currentCharIndex = 0;
	textSpeed = speed;
	isFinished = false;
	active = true;

	const int maxTextWidth = SCREEN_WIDTH - (marginX * 2) - (paddingX * 2);
	FONT *fontObj = CommonGUI::Instance().GetFont(0);

	std::string currentLine = "";
	std::string word = "";

	for (size_t i = 0; i <= processedText.length(); i++)
	{
		char c = (i < processedText.length()) ? processedText[i] : '\0';
		if (c == ' ' || c == '\n' || c == '\0')
		{
			if (!word.empty())
			{
				std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
				if (text_length(fontObj, testLine.c_str()) > maxTextWidth)
				{
					lines.push_back(currentLine);
					currentLine = word;
				}
				else
					currentLine = testLine;
				word.clear();
			}

			if (c == '\n')
			{
				lines.push_back(currentLine);
				currentLine.clear();
			}
		}
		else
			word += c;
	}

	if (!currentLine.empty())
		lines.push_back(currentLine);
}

void DialogBox::Update()
{
	if (!active || isFinished)
		return;

	if (currentCharIndex < fullText.length())
	{
		currentCharIndex += textSpeed;
		if (currentCharIndex >= fullText.length())
		{
			currentCharIndex = fullText.length();
			isFinished = true;
		}
	}
	else
		isFinished = true;
}

void DialogBox::Draw(BITMAP *dest, int scrollXOffset, int scrollYOffset)
{
	if (!active)
		return;

	BITMAP *guiSheet = CommonGUI::Instance().GetBitmap();
	int x = scrollXOffset + marginX;
	int y = scrollYOffset + SCREEN_HEIGHT - boxHeight - marginY;
	int width = SCREEN_WIDTH - (marginX * 2);
	int height = boxHeight;

	if (guiSheet)
	{
		masked_blit(guiSheet, dest, 0, 0, x, y, TILE_SIZE, TILE_SIZE);
		masked_blit(guiSheet, dest, TILE_SIZE * 2, 0, x + width - TILE_SIZE, y, TILE_SIZE, TILE_SIZE);
		masked_blit(guiSheet, dest, 0, TILE_SIZE * 2, x, y + height - TILE_SIZE, TILE_SIZE, TILE_SIZE);
		masked_blit(guiSheet, dest, TILE_SIZE * 2, TILE_SIZE * 2, x + width - TILE_SIZE, y + height - TILE_SIZE, TILE_SIZE, TILE_SIZE);

		for (int px = x + TILE_SIZE; px < x + width - TILE_SIZE; px += TILE_SIZE)
		{
			int drawWidth = std::min(TILE_SIZE, (x + width - TILE_SIZE) - px);
			masked_blit(guiSheet, dest, TILE_SIZE, 0, px, y, drawWidth, TILE_SIZE);
			masked_blit(guiSheet, dest, TILE_SIZE, TILE_SIZE * 2, px, y + height - TILE_SIZE, drawWidth, TILE_SIZE);
		}

		for (int py = y + TILE_SIZE; py < y + height - TILE_SIZE; py += TILE_SIZE)
		{
			int drawHeight = std::min(TILE_SIZE, (y + height - TILE_SIZE) - py);
			masked_blit(guiSheet, dest, 0, TILE_SIZE, x, py, TILE_SIZE, drawHeight);
			masked_blit(guiSheet, dest, TILE_SIZE * 2, TILE_SIZE, x + width - TILE_SIZE, py, TILE_SIZE, drawHeight);
		}

		for (int py = y + TILE_SIZE; py < y + height - TILE_SIZE; py += TILE_SIZE)
		{
			int drawHeight = std::min(TILE_SIZE, (y + height - TILE_SIZE) - py);
			for (int px = x + TILE_SIZE; px < x + width - TILE_SIZE; px += TILE_SIZE)
			{
				int drawWidth = std::min(TILE_SIZE, (x + width - TILE_SIZE) - px);
				masked_blit(guiSheet, dest, TILE_SIZE, TILE_SIZE, px, py, drawWidth, drawHeight);
			}
		}
	}

	FONT *fontObj = CommonGUI::Instance().GetFont(0);
	int textX = x + paddingX;
	int textY = y + paddingY;

	size_t remainingChars = currentCharIndex;
	for (size_t i = 0; i < lines.size(); i++)
	{
		if (remainingChars == 0)
			break;

		std::string visibleText = lines[i];
		if (visibleText.length() > remainingChars)
		{
			visibleText = visibleText.substr(0, remainingChars);
			remainingChars = 0;
		}
		else
		{
			remainingChars -= visibleText.length();
			if (remainingChars > 0)
				remainingChars--;
		}

		textout_ex(dest, fontObj, visibleText.c_str(), textX, textY, textColor, -1);
		textY += lineHeight;
	}
}

void DialogBox::Advance()
{
	if (!active)
		return;

	if (!isFinished)
	{
		currentCharIndex = fullText.length();
		isFinished = true;
	}
	else
		active = false;
}
