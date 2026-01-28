#include "Game.h"

// Access Point To The Game
int main()
{
	Game game("OpenGL tutorial", 1280, 720, 4, 5, false);

	// LOOP
	while (!game.getWindowShouldClose())
	{
		// Update input
		game.update();
		game.render();
	}

	return 0;
}