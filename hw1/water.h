#pragma once
#pragma once
#include "stdafx.h"
#include "glad\glad.h"
#include "include\SDL.h"
#include "include\SDL_opengl.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <iostream>
#include <string>
#include <math.h>
#include <list>
#include "objLoader.h"
using namespace std;

struct water{
	glm::vec3 position;
	GLfloat uh;
	glm::vec3 color;
	glm::vec3 normal;
};

water** shallowWater(water** sw, GLfloat dt, GLfloat dx, GLuint nx);

void sendWaterToBuffer(water** sw, GLuint nx, bufferID waterBufferID, GLuint shader);

void displayWater(GLuint nx, bufferID waterBufferID, GLuint shader);