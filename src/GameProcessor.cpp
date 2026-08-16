#include "GameProcessor.h"
#include "Core/CommonGUI.h"
#include "Core/PathDatabase.h"
#include "Core/AssetPaths.h"


void GameProcessor::InitializeGame()
{
	if (allegro_init() != 0)
		exit(0);

	install_keyboard();

	if (!GameDatabase::Instance().Load(AssetPaths::GAMEDB))
	{
		set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
		allegro_message("Unable to load GAMEDB.BIN\n");
		exit(0);
	}

	StringDatabase::Instance().Load(AssetPaths::STRINGS);
	PathDatabase::Instance().Load(AssetPaths::PATHS);

	set_color_depth(8);
	if (set_gfx_mode(GFX_MODEX, SCREEN_WIDTH, SCREEN_HEIGHT, SCROLLABLE_VIRTUAL_W, SCROLLABLE_VIRTUAL_H) != 0)
	{
		if (set_gfx_mode(GFX_MODEX, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0) != 0)
		{
			set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
			allegro_message("Unable to set any graphic mode\n%s\n", allegro_error);
			exit(0);
		}
	}

	reserve_voices(AUDIO_MAX_VOICES, -1);
	if (install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL) < 0)
	{
		printf("Error initializing sound card");
		exit(0);
	}

	int pageW = VSCREEN_W;
	int pageH = VSCREEN_H;
	videoPages[0] = create_video_bitmap(pageW, pageH);
	videoPages[1] = create_video_bitmap(pageW, pageH);

	if (!videoPages[0] || !videoPages[1])
	{
		set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
		allegro_message("Out of memory: Could not create back buffers.");
		exit(0);
	}

	clear_to_color(videoPages[0], 0);
	clear_to_color(videoPages[1], 0);
	activePage = 0;

	show_video_bitmap(videoPages[activePage]);
}

void GameProcessor::ProcessEvents()
{
	InputManager::Instance().Update();
	currentState->AcquireInput(this);
}

void GameProcessor::Render()
{
	currentState->Render(this);
	FlipPages();
}

void GameProcessor::DeallocateResources()
{
	if (currentState)
	{
		currentState->UnloadResources();
		currentState.reset();
	}

	while (!stateStack.empty())
	{
		stateStack.back()->UnloadResources();
		stateStack.pop_back();
	}

	if (videoPages[0])
	{
		destroy_bitmap(videoPages[0]);
		videoPages[0] = nullptr;
	}
	
	if (videoPages[1])
	{
		destroy_bitmap(videoPages[1]);
		videoPages[1] = nullptr;
	}

	set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
}

void GameProcessor::ChangeState(std::unique_ptr<BaseState> newState)
{
	if (currentState)
		currentState->UnloadResources();
	currentState = std::move(newState);
	currentState->InitState(this);
}

void GameProcessor::PushState(std::unique_ptr<BaseState> newState)
{
	currentState->Pause();
	stateStack.push_back(std::move(currentState));
	currentState = std::move(newState);
	currentState->InitState(this);
}

void GameProcessor::PopState()
{
	currentState->UnloadResources();
	currentState.reset();

	if (!stateStack.empty())
	{
		currentState = std::move(stateStack.back());
		stateStack.pop_back();
		currentState->Resume();
	}
}

std::unique_ptr<BaseState> GameProcessor::CreateStateByName(const std::string &name)
{
	auto it = stateRegistry.find(name);
	if (it != stateRegistry.end())
		return it->second();
	return nullptr;
}

void GameProcessor::FlipPages()
{
	activePage ^= 1;
	scroll_screen(videoPages[activePage]->x_ofs + scrollX, videoPages[activePage]->y_ofs + scrollY);
}

void GameProcessor::HandleEvents()
{
	UpdateFade();
	if (currentState)
		currentState->ProcessInput(this);
}

void GameProcessor::FadeOut(int speed)
{
	memcpy(sourcePal, CommonGUI::Instance().GetPalette(), sizeof(PALETTE));
	memset(targetPal, 0, sizeof(PALETTE));
	fadeStep = 0;
	fadeSpeed = (speed > 0) ? speed : DEFAULT_FADE_SPEED;
	fadeDirection = FadeDirection::FADE_OUT;
	isFading = true;
}

void GameProcessor::FadeIn(int speed)
{
	memset(sourcePal, 0, sizeof(PALETTE));
	memcpy(targetPal, CommonGUI::Instance().GetPalette(), sizeof(PALETTE));
	set_palette(sourcePal);
	fadeStep = 0;
	fadeSpeed = (speed > 0) ? speed : DEFAULT_FADE_SPEED;
	fadeDirection = FadeDirection::FADE_IN;
	isFading = true;
}

void GameProcessor::UpdateFade()
{
	if (!isFading)
		return;

	fadeStep += fadeSpeed;
	if (fadeStep >= VGA_PALETTE_STEPS)
	{
		fadeStep = VGA_PALETTE_STEPS;
		isFading = false;
		set_palette(targetPal);
		fadeDirection = FadeDirection::NONE;
	}
	else
	{
		PALETTE tempPal;
		fade_interpolate(sourcePal, targetPal, tempPal, fadeStep, 0, VGA_PALETTE_SIZE - 1);
		set_palette(tempPal);
	}
}