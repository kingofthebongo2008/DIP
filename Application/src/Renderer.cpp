#include <iostream>
#include "Renderer.h"

Renderer::Renderer()
: running(true),
  winHeight(600),
  winWidth(800)
{}

void Renderer::init()
{
	initSFML();
	initOpenGL();
	initCamera();
	initShaders();
	initGeometry();
	//more inits
}

//----------------------------------------------------------------------------
//MAIN LOOP
void Renderer::run()
{
	while(running)
	{
		//poll events
		sf::Event event;
		while(window.pollEvent(event))
		{
			switch(event.type)
			{
				//window events
			case sf::Event::Closed:
				running = false;
				break;
			case sf::Event::Resized:
				winWidth = event.size.width;
				winHeight = event.size.height;
				glViewport(0,0,winWidth, winHeight);
				//TODO: Camera - rebuild projection matrix
				break;
				//keyboard
			case sf::Event::KeyPressed:
				handleKeyPressed(event);
				break;
				//mouse
			case sf::Event::MouseWheelMoved:
				handleMouseWheel(event);
				break;
				//unhandled
			default:
				break;
			}
		}//pollEvent

		//TODO: kontrola ci treba kreslit
		draw();
		window.display();
	}//main loop
}

//----------------------------------------------------------------------------
//Inits
//camera creation and setup
void Renderer::initCamera()
{
	camera.create();	//create default camera
}

//import and convert geometry
void Renderer::initGeometry()
{
	std::cout << "loading models..." << std::endl;
	teapot.import("./Application/data/teapot.obj");
	//teapot.generateVAOs(sampleAttribs, sampleShader);
}

//GLEW and quad initialisation
void Renderer::initOpenGL()
{
	glewExperimental = GL_TRUE;
	glewInit();

	//OGL inits
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	//TODO: make quad
}

//SFML initialisation
void Renderer::initSFML()
{
	sf::ContextSettings settings;
	settings.majorVersion = 3;
	settings.minorVersion = 3;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 4;

	window.create(sf::VideoMode(winWidth, winHeight),
				  "DIP",
				  sf::Style::Default,
				  settings);
}

//Shader init
void Renderer::initShaders()
{
	//sample
	sampleShader.create();
	sampleShader.addShader(VS, "Application/shaders/sample.vs");
	sampleShader.addShader(FS, "Application/shaders/sample.fs");
	sampleShader.link();
	sampleShader.initUniforms();
	sampleAttribs.posAttrib = glGetAttribLocation(sampleShader.getId(), "position");
	sampleAttribs.normAttrib = glGetAttribLocation(sampleShader.getId(), "normal");
}

//----------------------------------------------------------------------------
//Rendering
void Renderer::draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	sampleShader.use();
	setUniform(sampleShader.view, camera.getViewMatrix());
	setUniform(sampleShader.proj, camera.getProjectionMatrix());
	setUniform(sampleShader.world, teapot.getWorldMatrix());
	//teapot.draw(sampleShader);
	teapot.draw();
}

//----------------------------------------------------------------------------
//IO event handling
//Keyboard
void Renderer::handleKeyPressed(sf::Event & event)
{
	glm::vec3& o = camera.getOrigin();
	
	switch(event.key.code)
	{
	case sf::Keyboard::Escape:
		running = false;
		break;
	case sf::Keyboard::Down:
		o.z += 1.0f;
		camera.move();
		break;
	case sf::Keyboard::Up:
		o.z -= 1.0f;
		camera.move();
		break;
	case sf::Keyboard::Left:
		o.x -= 1.0f;
		camera.move();
		break;
	case sf::Keyboard::Right:
		o.x += 1.0f;
		camera.move();
		break;
	case sf::Keyboard::Add:
		o.y += 1.0f;
		camera.move();
		break;
	case sf::Keyboard::Subtract:
		o.y -= 1.0f;
		camera.move();
		break;
	default:
		break;
	}
}

//Mouse
void Renderer::handleMouseWheel(sf::Event & event)
{
	//TODO: change rotation based on mouse.wheel.delta
}