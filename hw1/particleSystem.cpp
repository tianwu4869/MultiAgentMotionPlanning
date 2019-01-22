#include "stdafx.h"
#include "glad\glad.h"
#include "include\SDL.h"
#include "include\SDL_opengl.h"

//Include order can matter here

#include <cstdio>

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
#include <Windows.h>
#include <mmsystem.h>
#include "particleSystem.h"
#include <omp.h>
#include <time.h>
#include <chrono>
#include <vector>
using namespace std;
using namespace std::chrono;

particle bouncingInteraction(particle water, GLfloat dt) {
	glm::vec3 nextPosition, nextVelocity;
	nextVelocity = water.velocity + glm::vec3(0, -9.8f, 0) * dt;
	nextPosition = water.position + nextVelocity * dt;
	if (nextPosition.y < 0) {
		nextVelocity.y *= -0.2f;
		water.velocity = nextVelocity;
	}
	else {
		water.velocity = nextVelocity;
		water.position = nextPosition;
	}
	return water;
}

particle continuousCollisionDetection(particle water, GLfloat dt, glm::vec3 targetPosition, GLfloat radius) {
	glm::vec3 nextPosition, nextVelocity;
	nextVelocity = water.velocity + glm::vec3(0, -9.8f, 0) * dt;
	nextPosition = water.position + nextVelocity * dt;
	glm::vec3 l = nextPosition - water.position;
	GLfloat l_norm2 = sqrtf(pow(l.x, 2) + pow(l.y, 2) + pow(l.z, 2));
	float a, b, c, delta, t, t_module;
	a = pow(l.x, 2) + pow(l.y, 2) + pow(l.z, 2);
	b = 2 * (water.position.x - targetPosition.x) * l.x + 2 * (water.position.y - targetPosition.y) * l.y + 2 * (water.position.z - targetPosition.z) * l.z;
	c = pow(water.position.x - targetPosition.x, 2) + pow(water.position.y - targetPosition.y, 2) + pow(water.position.z - targetPosition.z, 2) - pow(radius, 2);
	delta = pow(b, 2) - 4 * a * c;
	if (delta >= 0) {
		t = (-b - sqrt(delta)) / (2 * a);
		t_module = sqrt(pow(t * l.x, 2) + pow(t * l.y, 2) + pow(t * l.z, 2));
		if (t > 0 && t_module < l_norm2) {
			glm::vec3 towardCenterVelocity;
			towardCenterVelocity = targetPosition - water.position;
			GLfloat norm2 = sqrtf(pow(towardCenterVelocity.x, 2) + pow(towardCenterVelocity.y, 2) + pow(towardCenterVelocity.z, 2));
			glm::vec3 projectionTowardCenter;
			projectionTowardCenter = (water.velocity * towardCenterVelocity / pow(norm2, 2)) * towardCenterVelocity;
			glm::vec3 restVelocity;
			restVelocity = water.velocity - projectionTowardCenter;
			projectionTowardCenter *= -1.0f;
			water.velocity = restVelocity + projectionTowardCenter;
			//cout << water.position.y << endl;
		}
	}
	else {
		water.velocity = nextVelocity;
		water.position = nextPosition;
	}
	return water;
}

list<particle> initialFountain(list<particle> particles, GLuint num_particles, glm::vec3 birthplace, glm::vec3 color, GLfloat lifespan, GLfloat fountainScale) {
	
	GLfloat x_offset[8];
	GLfloat z_offset[8];
	GLfloat y_offset[8];
	particle newParticle[8];
	int coreNum = omp_get_num_procs();
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

	//#pragma omp parallel for
	for (int i = 0; i < num_particles; i++) {
		int k = omp_get_thread_num();
		x_offset[k] = (rand() % 1000) / 500.0f - 1.0f;
		z_offset[k] = (rand() % 1000) / 500.0f - 1.0f;
		y_offset[k] = (rand() % 300) / 1000.0f;
		x_offset[k] *= fountainScale;
		z_offset[k] *= fountainScale;
		newParticle[k].position = birthplace + glm::vec3(0, y_offset[k], 0);
		newParticle[k].color = color;
		newParticle[k].velocity = glm::vec3(x_offset[k], 5.0f - y_offset[k], z_offset[k]);
		newParticle[k].lifespan = lifespan;
		newParticle[k].age = 0;
		particles.push_back(newParticle[k]);
		//cout << k <<' '<<i<< endl;
	}
	auto duration = duration_cast<microseconds>(t2 - t1).count();

	//cout << duration <<endl;
	return particles;
}

list<particle> initialFire(list<particle> particles, GLuint num_particles, glm::vec3 birthplace, glm::vec3 color, GLfloat lifespan) {
	GLfloat x_offset, z_offset;
	particle newParticle;
	for (int i = 0; i < num_particles; i++) {
		float radius, angle;
		radius = (rand() % 300) / 1000.0f;
		radius = sqrtf(radius / 0.3) * 0.3;
		angle = (rand() % 1000) / 1000.0f * 2.0f * 3.1416f;
		x_offset = sin(angle) * radius;
		z_offset = cos(angle) * radius;
		newParticle.position = birthplace + glm::vec3(x_offset, 0, z_offset);
		newParticle.color = color;
		newParticle.color.g *= sqrt((0.3f - radius) / 0.3f);
		newParticle.velocity = glm::vec3(z_offset, 1.0f, x_offset);
		newParticle.lifespan = lifespan * (1.0f - ((rand() % 1000) / 1000.0f - 0.3f)) * pow(1.0f - radius, 2);
		newParticle.age = 0;
		particles.push_back(newParticle);
	}
	return particles;
}

list<particle> initialFireOnCloth(list<particle> particles, GLuint num_particles, vector<glm::vec3> fireBirthplace, glm::vec3 color, GLfloat lifespan) {
	num_particles = 1;
	if (particles.size() > 150) {
		num_particles = 0;
	}
	GLfloat x_offset, z_offset;
	particle newParticle;
	vector<glm::vec3>::iterator fp = fireBirthplace.begin();
	for (; fp != fireBirthplace.end(); fp++) {
		for (int i = 0; i < num_particles; i++) {
			float radius, angle;
			radius = (rand() % 100) / 1000.0f;
			radius = sqrtf(radius / 0.1) * 0.1;
			angle = (rand() % 1000) / 1000.0f * 2.0f * 3.1416f;
			x_offset = sin(angle) * radius;
			z_offset = cos(angle) * radius;
			newParticle.position = *fp + glm::vec3(x_offset, 0, z_offset);
			newParticle.color = color;
			newParticle.color.g *= sqrt((0.1f - radius) / 0.1f);
			newParticle.velocity = glm::vec3(z_offset, 1.0f, x_offset);
			newParticle.lifespan = lifespan * (1.0f - ((rand() % 1000) / 1000.0f - 0.1f)) * pow(1.0f - radius, 2);
			newParticle.age = 0;
			particles.push_back(newParticle);
		}
	}
	return particles;
}

list<particle> initialExplosion(list<particle> particles, GLuint num_particles, glm::vec3 birthplace, glm::vec3 color, GLfloat lifespan) {
	GLfloat x_offset, z_offset, y_offset;
	particle newParticle;
	glm::vec3 changedColor((rand() % 1000) / 1000.0f, (rand() % 1000) / 1000.0f, (rand() % 1000) / 1000.0f);
	for (int i = 0; i < num_particles; i++) {
		float radius, angle;
		radius = (rand() % 500) / 1000.0f;
		radius = sqrtf(radius / 0.5) * 0.5;
		angle = (rand() % 1000) / 1000.0f * 2.0f * 3.1416f;
		x_offset = sin(angle) * radius;
		z_offset = cos(angle) * radius;
		y_offset = sqrtf(pow(0.5f, 2) - pow(x_offset, 2) - pow(z_offset, 2));
		
		/*if (radius < (0.3f / 3)) {
			y_offset *= (-1.0f);
		}*/
		newParticle.position = birthplace;
		newParticle.color = changedColor;
		newParticle.velocity = glm::vec3(x_offset, y_offset, z_offset) * 4.0f;
		newParticle.lifespan = lifespan;
		newParticle.age = 0;
		particles.push_back(newParticle);
	}
	return particles;
}

list<particle> manageFountain(list<particle> particles, GLuint generationRate, GLfloat dt, glm::vec3 birthplace, glm::vec3 color, GLfloat lifespan, GLfloat fountainScale, glm::vec3 targetPosition, GLfloat radius) {
	list<particle>::iterator it = particles.begin();

	//Update ages of all particles; Erase particles supposed to disappear.
	for (; it != particles.end();) {	
		if ((it->age + dt) < it->lifespan) {
			it->age += dt;
			it++;
		}
		else {
			it = particles.erase(it);
		}
	}

	//Push new bunches of particles into list
	particles = initialFountain(particles, int(generationRate * dt), birthplace, color, lifespan, fountainScale);
	
	//Update velocity and position of all particles
	it = particles.begin();
	for (; it != particles.end(); it++) {
		*it = bouncingInteraction(*it, dt);
		*it = continuousCollisionDetection(*it, dt, targetPosition, radius);
	}
	//cout << targetPosition.y << endl;
	return particles;
}

list<particle> manageFire(list<particle> particles, GLuint generationRate, GLfloat dt, glm::vec3 birthplace, glm::vec3 color, GLfloat lifespan) {
	list<particle>::iterator it = particles.begin();

	//Update ages of all particles; Erase particles supposed to disappear.
	for (; it != particles.end();) {
		if ((it->age + dt) < it->lifespan) {
			it->age += dt;
			it++;
		}
		else {
			it = particles.erase(it);
		}
	}

	//Push new bunches of particles into list
	particles = initialFire(particles, int(generationRate * dt), birthplace, color, lifespan);

	//Update velocity and position of all particles
	it = particles.begin();
	for (; it != particles.end(); it++) {
		it->position += it->velocity * dt;
		it->color.g -= color.g * dt;
		//cout << it->position.x << ' ' << it->position.y << ' ' << it->position.z << endl;
	}
	return particles;
}

list<particle> manageFireOnCloth(list<particle> particles, GLuint generationRate, GLfloat dt, vector<glm::vec3> fireBirthplace, glm::vec3 color, GLfloat lifespan) {
	list<particle>::iterator it = particles.begin();

	//Update ages of all particles; Erase particles supposed to disappear.
	for (; it != particles.end();) {
		if ((it->age + dt) < it->lifespan) {
			it->age += dt;
			it++;
		}
		else {
			it = particles.erase(it);
		}
	}

	//Push new bunches of particles into list
	particles = initialFireOnCloth(particles, int(generationRate * dt), fireBirthplace, color, lifespan);

	//Update velocity and position of all particles
	it = particles.begin();
	for (; it != particles.end(); it++) {
		it->position += it->velocity * dt;
		it->color.g -= color.g * dt;
		//cout << it->position.x << ' ' << it->position.y << ' ' << it->position.z << endl;
	}
	return particles;
}

list<particle> manageFireworks(list<particle> particles, GLfloat dt) {
	list<particle>::iterator it = particles.begin();
	
	//Update ages of all particles; Erase particles supposed to disappear.
	for (; it != particles.end(); ) {
		if (it->age != 666) {
			if ((it->age + dt) < it->lifespan) {
				it->age += dt;
				it++;
			}
			else {
				it = particles.erase(it);
			}
		}
		else {
			it++;
		}
	}

	//Update velocity and position of all particles
	it = particles.begin();
	for (; it != particles.end(); it++) {
		if (it->age != 666) {
			it->velocity += glm::vec3(0, -4.9f * dt, 0);
			it->position += it->velocity * dt;
		}
	}
	
	return particles;
}

void sendDataToBuffer(list<particle> particles, GLuint vao, GLuint vbo, GLuint particleShader) {
	GLuint num_particles;
	num_particles = particles.size();
	
	GLfloat *particleAttributes = new GLfloat[6 * num_particles];
	list<particle>::iterator it = particles.begin();
	for (int i = 0; it != particles.end(); it++, i++) {
		particleAttributes[6 * i] = it->position.x;
		particleAttributes[6 * i + 1] = it->position.y;
		particleAttributes[6 * i + 2] = it->position.z;
		particleAttributes[6 * i + 3] = it->color.r;
		particleAttributes[6 * i + 4] = it->color.g;
		particleAttributes[6 * i + 5] = it->color.b;
	}
	
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, num_particles * 6 * sizeof(float), particleAttributes, GL_STREAM_DRAW);

	int posAttrib = glGetAttribLocation(particleShader, "position");
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(posAttrib);

	int colAttrib = glGetAttribLocation(particleShader, "inColor");
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(colAttrib);

	glBindVertexArray(0);

	delete particleAttributes;
}

list<particle> blastOff(list<particle> firework, GLfloat dt, glm::vec3 velocity, glm::vec3 birthplace, glm::vec3 color, GLfloat lifespan, GLuint num_particles) {
	
	if (firework.size() == 0) {
		PlaySound(TEXT("blast-off.wav"), NULL, SND_ASYNC | SND_FILENAME);
		//cout << firework.size();
		GLfloat x_offset, y_offset, z_offset;
		x_offset = (rand() % 400) / 1000.0f - 0.2f;
		y_offset = (rand() % 400) / 1000.0f - 0.2f;
		z_offset = (rand() % 400) / 1000.0f - 0.2f;
		particle firecracker;
		firecracker.position = birthplace;
		firecracker.velocity = velocity + glm::vec3(x_offset, y_offset, z_offset);
		firecracker.color = color;
		firecracker.age = 666;
		firework.push_front(firecracker);
		return firework;
	}
	list<particle>::iterator it = firework.begin();
	for (; (it != firework.end()) && (it->age == 666);) {
		
		if (it->velocity.y > 0) {
			it->velocity += glm::vec3(0, -1.0f * dt, 0);
			it->position += it->velocity * dt;
			it++;
		}
		else {
			//PlaySound(TEXT("blast-off.wav"), NULL, SND_ASYNC | SND_FILENAME);
			GLfloat x_offset;
			x_offset = (rand() % 400) / 1000.0f - 0.2f;
			particle firecracker;
			firecracker.position = birthplace;
			firecracker.velocity = velocity + glm::vec3(x_offset, 0, 0);
			firecracker.color = color;
			firecracker.age = 666;
			glm::vec3 explosionPosition;
			glm::vec3 previousColor;
			explosionPosition = it->position;
			previousColor = it->color;
			firework.erase(it);
			//firework.push_back(firecracker);
			firework = initialExplosion(firework, num_particles, explosionPosition, previousColor, lifespan);
			PlaySound(TEXT("explosion.wav"), NULL, SND_ASYNC | SND_FILENAME);
			return firework;
		}

	}
	GLfloat blastOffNewFirework;
	blastOffNewFirework = (rand() % 100) / 100.0f;
	if (blastOffNewFirework == 0.02f) {
		GLfloat x_offset, y_offset, z_offset;
		x_offset = (rand() % 800) / 1000.0f - 0.4f;
		y_offset = (rand() % 800) / 1000.0f - 0.4f;
		z_offset = (rand() % 800) / 1000.0f - 0.4f;
		particle firecracker;
		firecracker.position = birthplace;
		firecracker.velocity = velocity + glm::vec3(x_offset, y_offset, z_offset);
		firecracker.color = color;
		firecracker.age = 666;
		firework.push_front(firecracker);
	}
	return firework;
}