#pragma once
#include "stdafx.h"
#include "glad\glad.h"
#include "include\SDL.h"
#include "include\SDL_opengl.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <time.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <list>
#include <mmsystem.h>
#include <vector>
#include "water.h"
#include "objLoader.h"
using namespace std; 

class obstacle {
public:
	virtual bool pointInside() { return 0; }
	virtual bool lineIntersection() { return 0; }
};

class circularObstacle : public obstacle {
public:
	glm::vec3 center;
	GLfloat radius;
	bool pointInside(glm::vec3 point) {
		glm::vec3 distance;
		distance = point - center;
		GLfloat d;
		d = sqrtf(glm::dot(distance, distance));
		if (d <= radius) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	bool lineIntersection(glm::vec3 point1, glm::vec3 point2) {
		glm::vec3 distance;
		distance = point1 - point2;
		GLfloat d;
		d = sqrtf(glm::dot(distance, distance));
		float a, b, c, delta, t, t_module;
		a = glm::dot(distance, distance);
		b = 2 * glm::dot(point2 - center, distance);
		c = glm::dot(point2 - center, point2 - center) - pow(radius, 2);
		delta = pow(b, 2) - 4 * a * c;
		if (delta < 0) {
			return FALSE;
		}
		else {
			t = (-b - sqrt(delta)) / (2 * a);
			t_module = sqrt(pow(t * distance.x, 2) + pow(t * distance.y, 2) + pow(t * distance.z, 2));
			if (t > 0 && t_module < d) {
				return TRUE;
			}
			else {
				return FALSE;
			}
		}
	}
};

struct velocityCone {
	glm::vec2 truncatePoint1;
	glm::vec2 truncatePoint2;
	glm::vec2 apex;
};

struct stateOnRoute {
	glm::vec3 relativeAgentPosition;
	GLfloat relativeAngle;
	GLfloat agentDistance;
	GLfloat agentSpeed;
	glm::vec3 velocity;
	glm::vec3 position;
	glm::vec3 goal;
	vector<pair<int, velocityCone>> cone;
	GLfloat radius;
	vector<glm::vec3> PRM;
};

struct boid {
	glm::vec3 position;
	glm::vec3 velocity;
	GLfloat maxSpeed;
	GLfloat maxForce;
	int label;
	glm::vec3 acceleration;
	bufferID buffer;
};

void sendRoadToBuffer(vector<glm::vec3> path, bufferID roadBufferID, GLuint shader);

stateOnRoute computePositionOnRoute(vector<glm::vec3> path, stateOnRoute modelState);

//void renderWater(GLuint nodesNumber, bufferID roadBufferID, GLuint shader);

velocityCone computeTruncatePoints(stateOnRoute from, stateOnRoute to);

vector<stateOnRoute> RVO(vector<stateOnRoute> objectState, GLfloat dt, vector<stateOnRoute> obstacles, GLuint shader);

boid* boidMovement(boid* boids, GLfloat dt, GLfloat boidRouteRadius, GLfloat boidNumber, GLfloat separationRadius, GLfloat alignmentRadius, GLfloat cohesionRadius);

void renderBoids(boid* boids, float* boidsTriangle, GLfloat boidNumber, GLuint shader, GLuint* boidsIndex);

void renderCircleBeneathModel(vector<stateOnRoute> obj, vector<stateOnRoute> obstacles, GLuint shader, GLuint* circleIndex, bufferID circleBeneathModelBuffer);

vector<glm::vec3> PRM(stateOnRoute agent, vector<stateOnRoute> obstacles, GLuint sampleNumber);