#include "Light.h"

Light::Light()
: fov(60.0),
  nearPlane(0.75),
  farPlane(1000.0),
  origin(glm::vec3(150.0, 150.0, 0.0)),
  target(glm::vec3(0.0, 0.0, 0.0)),
  color(glm::vec4(1.0, 1.0, 1.0, 1.0))
 {}

void Light::create()
{
	computeProjectionMatrix();
	computeViewMatrix();
}

void Light::setOrigin(glm::vec3 & o)
{
	origin = o;
	computeViewMatrix();
}

void Light::setTarget(glm::vec3 & t)
{
	target = t;
	computeViewMatrix();
}

void Light::computeProjectionMatrix()
{
	projectionMatrix = glm::perspective(fov,
										(float)SHADOW_WIDTH/(float)SHADOW_HEIGHT,
										nearPlane,
										farPlane);
}

void Light::computeViewMatrix()
{
	viewMatrix = glm::lookAt(origin,
							 target,
							 up);
}

const glm::vec3 Light::up = glm::vec3(0.0, 1.0, 0.0);
glm::mat4 Light::biasMatrix = glm::mat4(0.5, 0.0, 0.0, 0.0,
					  				    0.0, 0.5, 0.0, 0.0,
    						 			0.0, 0.0, 0.5, 0.0,
    						 			0.5, 0.5, 0.5, 1.0);