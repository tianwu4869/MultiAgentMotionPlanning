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
using namespace std;

struct node {
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 color;
	glm::vec2 textureCoordinate;
	GLfloat ks;
	GLfloat kd;
	GLfloat verticalInterval;
	GLfloat horizontalInterval;
};

node** manageCloth(node** cloth, GLfloat dt, GLuint length, GLuint width, glm::vec3 spherePosition, GLfloat sphereRadius, glm::vec3 windSpeed);

vector<glm::vec3> sendClothToBuffer(node** cloth, GLuint shader, GLuint length, GLuint width, bufferID id, GLfloat burntRadius, glm::vec2 burntCenter);

int importTexture();

void displayCloth(int shaderProgram, bufferID id, GLuint length, GLuint width);
