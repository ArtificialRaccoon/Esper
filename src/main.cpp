#include "allegro.h"
#include "GameProcessor.h"
#include "Core/GameDefines.h"
#include "States/MapState.h"

volatile int ticks = 0;
volatile long absTicks = 0;  // For InputManager
void timer(void)
{
	ticks = ticks + 1;
	absTicks = absTicks + 1;
}
END_OF_FUNCTION(timer)

int main()
{
	GameProcessor objGame;
	objGame.InitializeGame();

	install_timer();
	LOCK_VARIABLE(ticks);
	LOCK_VARIABLE(absTicks);
	LOCK_FUNCTION(timer);
	install_int_ex(timer, BPS_TO_TIMER(TARGET_FPS));

	// Register all states before the first ChangeState
	objGame.RegisterState("MapState", [] { return std::make_unique<MapState>(); });

	objGame.ChangeState(std::move(objGame.CreateStateByName("MapState")));

	while (objGame.IsRunning())
	{
		while (ticks == 0)
			rest(1);

		while (ticks > 0)
		{
			int old_ticks = ticks;
			ticks = ticks - 1;
			if (old_ticks <= ticks)
				break;

			objGame.ProcessEvents();
			objGame.HandleEvents();
		}

		objGame.Render();
	}

	objGame.DeallocateResources();
	return 0;
}
END_OF_MAIN()