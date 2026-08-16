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

	int x = scrollXOffset + marginX;
	int y = scrollYOffset + SCREEN_HEIGHT - boxHeight - marginY;
	int width = SCREEN_WIDTH - (marginX * 2);
	int height = boxHeight;

	CommonGUI::Instance().DrawNineSliceBox(dest, x, y, width, height);

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

void DialogBox::Close()
{
	active = false;
	isFinished = true;
}
