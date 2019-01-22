#pragma once

#include "glad\glad.h"
#include "include\SDL.h"
#include "include\SDL_opengl.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <list>
#include <vector>
using namespace std;

class particle {
public:
	glm::vec3 color;
	glm::vec3 position;
	glm::vec3 velocity;
	GLfloat lifespan;
	GLfloat age;
};

list<particle> initialFountain(list<particle> particles, GLuint num_particles, glm::vec3 birthplace, glm::vec3 color, GLfloat lifespan, GLfloat fountainScale);

list<particle> initialFire(list<particle> particles, GLuint num_particles, glm::vec3 birthplace, glm::vec3 color, GLfloat lifespan);

list<particle> initialExplosion(list<particle> particles, GLuint num_particles, glm::vec3 birthplace, glm::vec3 color, GLfloat lifespan);

void sendDataToBuffer(list<particle> particles, GLuint vao, GLuint vbo, GLuint particleShader);

list<particle> manageFire(list<particle> particles, GLuint generationRate, GLfloat dt, glm::vec3 birthplace, glm::vec3 color, GLfloat lifespan);

list<particle> manageFireOnCloth(list<particle> particles, GLuint generationRate, GLfloat dt, vector<glm::vec3> fireBirthplace, glm::vec3 color, GLfloat lifespan);

list<particle> manageFireworks(list<particle> particles, GLfloat dt);

list<particle> manageFountain(list<particle> particles, GLuint generationRate, GLfloat dt, glm::vec3 birthplace, glm::vec3 color, GLfloat lifespan, GLfloat fountainScale, glm::vec3 targetPosition, GLfloat radius);

list<particle> blastOff(list<particle> firework, GLfloat dt, glm::vec3 velocity, glm::vec3 birthplace, glm::vec3 color, GLfloat lifespan, GLuint num_particles);
