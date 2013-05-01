#ifndef RENDERER_H
#define RENDERER_H

//GLEW
#define GLEW_STATIC
#include <GL/glew.h>
//SFML
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
//DIP
#include "Camera.h"

class Renderer
{
public:
	Renderer();
	void init();
	void run();
private:
	void handleKeyPressed(sf::Event & event);
	void handleMouseWheel(sf::Event & event);
	void initCamera();
	void initSFML();
	void initOpenGL();

	bool running;
	int winHeight;
	int winWidth;
	sf::RenderWindow window;

	Camera camera;
};

#endif