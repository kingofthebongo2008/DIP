#include <iostream>
#include "Renderer.h"
#include "quad.h"

Renderer::Renderer()
: running(true),
  winHeight(WIN_HEIGHT),
  winWidth(WIN_WIDTH),
  ry(0.0f)
{}

void Renderer::init(char* modelFile)
{
	initSFML();
	initOpenGL();
	initCamera();
	initLight();
	initShaders();
	initGeometry(modelFile);
	initQuad();
	initFramebuffers();
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
				camera.recomputeProjection(winWidth, winHeight);
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
//primary light source creation
void Renderer::initLight()
{
	light.create();		//create default light
}

//init framebuffer objects
void Renderer::initFramebuffers()
{
	glGenFramebuffers(NUM_FBOS, FBOs);
	//-------------------------------------------------------------------------
	// CAMERA POV RENDER
	//-------------------------------------------------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, FBOs[RENDER_FBO]);
	//textures
	glGenTextures(1, &renderDepthTex);
	glBindTexture(GL_TEXTURE_2D, renderDepthTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,
				 WIN_WIDTH, WIN_HEIGHT, 0,
				 GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	//create world-space coord texture
	glGenTextures(1, &renderWSCTex);
	glBindTexture(GL_TEXTURE_2D, renderWSCTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
	             WIN_WIDTH, WIN_HEIGHT, 0,
	             GL_RGBA, GL_FLOAT, 0);
	//create normal texture
	glGenTextures(1, &renderNormalTex);
	glBindTexture(GL_TEXTURE_2D, renderNormalTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F,
	             WIN_WIDTH, WIN_HEIGHT, 0,
	             GL_RGB, GL_FLOAT, 0);
	//create color texture
	glGenTextures(1, &renderColorTex);
	glBindTexture(GL_TEXTURE_2D, renderColorTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
				 WIN_WIDTH, WIN_HEIGHT, 0,
				 GL_RGBA, GL_FLOAT, 0);
	//attach textures to FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
	                       renderDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           renderWSCTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                           renderNormalTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
                           renderColorTex, 0);
	//set draw buffers
	GLenum renderMRT[] = {
		GL_COLOR_ATTACHMENT0,//wsc location0
		GL_COLOR_ATTACHMENT1,//normals location1
		GL_COLOR_ATTACHMENT2 //color location2
	};
	glDrawBuffers(3, renderMRT);
	//check
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    	std::cerr << "Cosi sa posralo render" << std::endl;
    //-------------------------------------------------------------------------
	// LIGHT POV RENDER
	//-------------------------------------------------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, FBOs[RSM_FBO]);
	//textures
	glGenTextures(1, &rsmDepthTex);
	glBindTexture(GL_TEXTURE_2D, rsmDepthTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
  	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,
				 WIN_WIDTH, WIN_HEIGHT, 0,
				 GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	//create world-space coord texture
	glGenTextures(1, &rsmWSCTex);
	glBindTexture(GL_TEXTURE_2D, rsmWSCTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
	             WIN_WIDTH, WIN_HEIGHT, 0,
	             GL_RGBA, GL_FLOAT, 0);
	//create normal texture
	glGenTextures(1, &rsmNormalTex);
	glBindTexture(GL_TEXTURE_2D, rsmNormalTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F,
	             WIN_WIDTH, WIN_HEIGHT, 0,
	             GL_RGB, GL_FLOAT, 0);
	//create color texture
	glGenTextures(1, &rsmColorTex);
	glBindTexture(GL_TEXTURE_2D, rsmColorTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
				 WIN_WIDTH, WIN_HEIGHT, 0,
				 GL_RGBA, GL_FLOAT, 0);
	//attach textures to FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
	                       rsmDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           rsmWSCTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                           rsmNormalTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
                           rsmColorTex, 0);
	//set draw buffers
	GLenum rsmMRT[] = {
		GL_COLOR_ATTACHMENT0,//wsc location0
		GL_COLOR_ATTACHMENT1,//normals location1
		GL_COLOR_ATTACHMENT2 //color location2
	};
	glDrawBuffers(3, rsmMRT);
	//check
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    	std::cerr << "Cosi sa posralo rsm" << std::endl;
    //-------------------------------------------------------------------------
	// DEFERRED SHADING
	//-------------------------------------------------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, FBOs[DEFERRED_FBO]);
	//generate the output color texture
	glGenTextures(1, &deferredColTex);
	glBindTexture(GL_TEXTURE_2D, deferredColTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
	             WIN_WIDTH, WIN_HEIGHT, 0,
	             GL_RGBA, GL_FLOAT, 0);
	//attach textures to FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
	                       renderDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           deferredColTex, 0);
	//set draw buffer
	GLenum deferredRT[] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, deferredRT);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    	std::cerr << "Cosi sa posralo deferred" << std::endl;
    //unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//import and convert geometry
void Renderer::initGeometry(char* modelFile)
{
	std::cout << "loading models..." << std::endl;
	model.import(modelFile);
	//model.scale();
	//model.moveBy(model.getCenter());
	camera.setTarget(model.getCenter());
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
}

//init fullscreen quad for rendering
void Renderer::initQuad()
{
	//generate VAO
	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);
	//generate buffers
	glGenBuffers(2, quadBuffers);
	//fill in vertices
	glBindBuffer(GL_ARRAY_BUFFER, quadBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices),
                 quadVertices, GL_STATIC_DRAW);
	//fill in indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadBuffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad),
                 quad, GL_STATIC_DRAW);
	//init attributes
	glEnableVertexAttribArray(0);	//enable position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QuadVertex),
                          (void*)offsetof(QuadVertex, position));
	glEnableVertexAttribArray(1);	//enable texcoord
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(QuadVertex),
                          (void*)offsetof(QuadVertex, texcoord));
	//unbind
	glBindVertexArray(0);
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
	renderShader.create();
	renderShader.addShader(VS, "Application/shaders/render.vs");
	renderShader.addShader(FS, "Application/shaders/render.fs");
	renderShader.link();
	renderShader.initUniforms();
	//simple quad rendering with single texture
	quadShader.create();
	quadShader.addShader(VS, "Application/shaders/quad.vs");
	quadShader.addShader(FS, "Application/shaders/quad.fs");
	quadShader.link();
	quadShader.initUniforms();
	//point rendering
	pointsShader.create();
	pointsShader.addShader(VS, "Application/shaders/points.vs");
	pointsShader.addShader(FS, "Application/shaders/points.fs");
	pointsShader.link();
	pointsShader.initUniforms();
	//deferred shader
	deferredShader.create();
	deferredShader.addShader(VS, "Application/shaders/quad.vs");
	deferredShader.addShader(FS, "Application/shaders/deferred.fs");
	deferredShader.link();
	deferredShader.initUniforms();
}

//----------------------------------------------------------------------------
//Rendering
void Renderer::draw()
{
	ry = 0.05f;
	model.rotate(ry, glm::vec3(0.0,1.0,0.0));
	//compute normal matrix for light
	glm::mat3 NormalMatrix = glm::transpose(glm::inverse(glm::mat3(light.getViewMatrix()*model.getWorldMatrix())));

	//if change
		//RSM
		glBindFramebuffer(GL_FRAMEBUFFER, FBOs[RSM_FBO]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderShader.use();
		setUniform(renderShader.view, light.getViewMatrix());
		setUniform(renderShader.proj, light.getProjectionMatrix());
		setUniform(renderShader.world, model.getWorldMatrix());
		setUniform(renderShader.normalMat, NormalMatrix);
		model.draw();
		//ISM
		//pull-push
	//-------------------------------------------------------------------------------
	//Render into G-buffer - Camera POV
	//-------------------------------------------------------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, FBOs[RENDER_FBO]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	NormalMatrix = glm::transpose(glm::inverse(glm::mat3(camera.getViewMatrix()*model.getWorldMatrix())));
	renderShader.use();
	setUniform(renderShader.view, camera.getViewMatrix());
	setUniform(renderShader.proj, camera.getProjectionMatrix());
	setUniform(renderShader.world, model.getWorldMatrix());
	setUniform(renderShader.normalMat, NormalMatrix);
	model.draw();
	//discontinuity
	//split
	//deferred shading
	glBindFramebuffer(GL_FRAMEBUFFER, FBOs[DEFERRED_FBO]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 DVP = Light::biasMatrix * light.getProjectionMatrix() * light.getViewMatrix();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderWSCTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, renderNormalTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, renderColorTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, rsmDepthTex);
	deferredShader.use();
	setUniform(deferredShader.wscTex, 0);
	setUniform(deferredShader.normalTex, 1);
	setUniform(deferredShader.colorTex, 2);
	setUniform(deferredShader.shadowTex, 3);
	setUniform(deferredShader.view, camera.getViewMatrix());
	setUniform(deferredShader.biasDVP, DVP);
	setUniform(deferredShader.cameraPos, camera.getOrigin());
	setUniform(deferredShader.lightPos, light.getOrigin());
	drawFullscreenQuad();
	//gather
	//blurX
	//blurY
	//-------------------------------------------------------------------------------
	// Show Result
	//-------------------------------------------------------------------------------
	//bind main FBO
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/*pointsShader.use();
	setUniform(pointsShader.viewMat, camera.getViewMatrix());
	setUniform(pointsShader.projMat, camera.getProjectionMatrix());
	setUniform(pointsShader.worldMat, model.getWorldMatrix());
	model.drawPointCloud();*/
	quadShader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, deferredColTex);
	setUniform(quadShader.tex, 0);
	//render quad
	drawFullscreenQuad();
}

void Renderer::drawFullscreenQuad()
{
	glBindVertexArray(quadVAO);
	glDrawElements(GL_TRIANGLES, sizeof(quad)/sizeof(**quad),
                   GL_UNSIGNED_SHORT, NULL);
	glBindVertexArray(0);
}

//----------------------------------------------------------------------------
//IO event handling
//Keyboard
void Renderer::handleKeyPressed(sf::Event & event)
{
	glm::vec3& o = light.getOrigin();
	
	switch(event.key.code)
	{
	case sf::Keyboard::Escape:
		running = false;
		break;
	case sf::Keyboard::Down:
		o.z += 1.0f;
		light.move();
		break;
	case sf::Keyboard::Up:
		o.z -= 1.0f;
		light.move();
		break;
	case sf::Keyboard::Left:
		o.x -= 1.0f;
		light.move();
		break;
	case sf::Keyboard::Right:
		o.x += 1.0f;
		light.move();
		break;
	case sf::Keyboard::Add:
		o.y += 1.0f;
		light.move();
		break;
	case sf::Keyboard::Subtract:
		o.y -= 1.0f;
		light.move();
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