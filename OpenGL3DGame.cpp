#include "Game.h"
#include "Constants.h"

// Access Point To The Game
int main()
{
	Game game(
		"OpenGL tutorial",
		Constants::Screen::DEFAULT_WIDTH,
		Constants::Screen::DEFAULT_HEIGHT,
		Constants::OpenGL::DEFAULT_GL_VERSION_MAJOR,
		Constants::OpenGL::DEFAULT_GL_VERSION_MINOR,
		false
	);

	// LOOP
	while (!game.getWindowShouldClose())
	{
		// Update input
		game.update();
		game.render();
	}

	return 0;
}