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
#include "objLoader.h"
#include "cloth.h"
#include "particleSystem.h"
using namespace std;

node sphereDetection(node point, glm::vec3 spherePosition, GLfloat sphereRadius) {
	GLfloat distance;
	glm::vec3 normal = point.position - spherePosition;
	distance = sqrtf(glm::dot(normal, normal));
	if (distance < sphereRadius + 0.09f) {
		normal /= distance;
		glm::vec3 bounce;
		bounce = glm::dot(point.velocity, normal) * normal;
		point.velocity -= 1.5f * bounce;
		point.position += (0.1f + sphereRadius - distance) * normal;
	}
	return point;
}

node* windDrag(node* triangle, GLuint length, GLuint width, glm::vec3 windSpeed, GLfloat dt) {
	glm::vec3 normal, velocity, dragForce;
	normal = glm::cross(triangle[0].position - triangle[1].position, triangle[1].position - triangle[2].position);
	velocity = (triangle[0].velocity + triangle[1].velocity + triangle[2].velocity) / 3.0f + windSpeed;
	GLfloat v, n;
	v = sqrtf(glm::dot(velocity, velocity));
	n = sqrtf(glm::dot(normal, normal));
	dragForce = 0.5f * v * glm::dot(velocity, normal) * normal / (2.0f * n);
	dragForce /= 3.0f;
	triangle[0].velocity = dragForce * dt;
	triangle[1].velocity = dragForce * dt;
	triangle[2].velocity = dragForce * dt;
	return triangle;
}

node** manageCloth(node** cloth, GLfloat dt, GLuint length, GLuint width, glm::vec3 spherePosition, GLfloat sphereRadius, glm::vec3 windSpeed) {
	node** clo = new node*[width];
	for (GLuint i = 0; i < width; i++) {
		clo[i] = new node[length];
	}
	#pragma omp parallel for
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < length; j++) {
			clo[i][j].velocity = cloth[i][j].velocity;
		}
	}
	#pragma omp parallel for
	for (int j = 0; j < length; j++) {// horizontal
		for (int i = 0; i < width - 1; i++) {
			glm::vec3 e;
			e = cloth[i][j].position - cloth[i + 1][j].position;
			GLfloat l;
			l = sqrt(glm::dot(e, e));
			e /= l;
			GLfloat v1, v2;
			v1 = glm::dot(e, cloth[i][j].velocity);
			v2 = glm::dot(e, cloth[i + 1][j].velocity);
			GLfloat fSpring;
			fSpring = -cloth[i][j].ks * (cloth[i][j].horizontalInterval - l) - cloth[i][j].kd * (v2 - v1);
			fSpring /= 2.0f;
			clo[i][j].velocity -= fSpring * e * dt;
			clo[i + 1][j].velocity += fSpring * e * dt;
		}
	}
	#pragma omp parallel for
	for (int i = 0; i < width; i++) {// vertical
		for (int j = 0; j < length - 1; j++) {
			glm::vec3 e;
			e = cloth[i][j].position - cloth[i][j + 1].position;
			GLfloat l;
			l = sqrt(glm::dot(e, e));
			e /= l;
			GLfloat v1, v2;
			v1 = glm::dot(e, cloth[i][j].velocity);
			v2 = glm::dot(e, cloth[i][j + 1].velocity);
			GLfloat fSpring;
			fSpring = -cloth[i][j].ks * (cloth[i][j].verticalInterval - l) - cloth[i][j].kd * (v2 - v1);
			clo[i][j].velocity -= fSpring * e * dt;
			clo[i][j + 1].velocity += fSpring * e * dt;
		}
	}

	//for (GLuint i = 0; i < (width - 1); i++) {// wind
	//	for (GLuint j = 0; j < (length - 1); j++) {
	//		node triangle[3];
	//		node* returnedTriangle;
	//		triangle[0] = cloth[i + 1][j];
	//		triangle[1] = cloth[i][j];
	//		triangle[2] = cloth[i][j + 1];
	//		returnedTriangle = windDrag(triangle, length, width, windSpeed, dt);
	//		clo[i + 1][j].velocity += returnedTriangle[0].velocity;
	//		clo[i][j].velocity += returnedTriangle[1].velocity;
	//		clo[i][j + 1].velocity += returnedTriangle[2].velocity;
	//		triangle[0] = cloth[i + 1][j];
	//		triangle[1] = cloth[i][j + 1];
	//		triangle[2] = cloth[i + 1][j + 1];
	//		returnedTriangle = windDrag(triangle, length, width, windSpeed, dt);
	//		clo[i + 1][j].velocity += returnedTriangle[0].velocity;
	//		clo[i][j + 1].velocity += returnedTriangle[1].velocity;
	//		clo[i + 1][j + 1].velocity += returnedTriangle[2].velocity;
	//	}
	//}

	#pragma omp parallel for
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < length; j++) {
			if (j == 0) {
				cloth[i][j].velocity = glm::vec3(0, 0, 0);
			}
			else {
				cloth[i][j].velocity = clo[i][j].velocity;
				cloth[i][j].velocity += glm::vec3(0, -9.8f, 0) * dt;
			}
			cloth[i][j].position += cloth[i][j].velocity * dt;
			//cloth[i][j] = sphereDetection(cloth[i][j], spherePosition, sphereRadius);
		}
	}

	for (GLuint i = 0; i < width; i++) {
		delete clo[i];
	}
	delete clo;
	return cloth;
}

GLuint burntCheck(GLfloat burntRadius, glm::vec2 burntCenter, glm::vec2* triangle) {
	//return false;
	GLfloat distance[3];
	distance[0] = sqrtf(pow(burntCenter.x - triangle[0].x, 2) + pow(burntCenter.y - triangle[0].y, 2));
	distance[1] = sqrtf(pow(burntCenter.x - triangle[1].x, 2) + pow(burntCenter.y - triangle[1].y, 2));
	distance[2] = sqrtf(pow(burntCenter.x - triangle[2].x, 2) + pow(burntCenter.y - triangle[2].y, 2));
	if (distance[0] < burntRadius & distance[1] < burntRadius & distance[2] < burntRadius) {
		return -1;
	}
	else if (distance[0] < burntRadius || distance[1] < burntRadius || distance[2] < burntRadius) {
		return 0;
	}
	else {
		return 1;
	}
}

vector<glm::vec3> sendClothToBuffer(node** cloth, GLuint shader, GLuint length, GLuint width, bufferID id, GLfloat burntRadius, glm::vec2 burntCenter) {
	vector<glm::vec3> fireBirthplace;
	GLuint* indices = new GLuint[(length - 1) * (width - 1) * 2 * 3];
	GLfloat* triangleCloth = new GLfloat[length * width * 5];

	GLuint count = 0;
	for (GLuint i = 0; i < (width - 1); i++) {
		for (GLuint j = 0; j < (length - 1); j++) {
			glm::vec2 triangle[3];
			triangle[0] = glm::vec2(i, j);
			triangle[1] = glm::vec2(i, j + 1);
			triangle[2] = glm::vec2(i + 1, j);
			GLuint burntSituation;
			burntSituation = burntCheck(burntRadius, burntCenter, triangle);
			if (1 == burntSituation) {
				indices[count] = i * length + j;
				indices[count + 1] = i * length + j + 1;
				indices[count + 2] = (i + 1) * length + j;
				count += 3;
			}
			else if (0 == burntSituation) {
				glm::vec3 fireOrigin;
				fireOrigin = (cloth[i][j].position + cloth[i][j + 1].position + cloth[i + 1][j].position) / 3.0f;
				fireBirthplace.push_back(fireOrigin);
			}
			triangle[0] = glm::vec2(i, j + 1);
			triangle[1] = glm::vec2(i + 1, j);
			triangle[2] = glm::vec2(i + 1, j + 1);
			burntSituation = burntCheck(burntRadius, burntCenter, triangle);
			if (1 == burntSituation) {
				indices[count] = i * length + j + 1;
				indices[count + 1] = (i + 1) * length + j;
				indices[count + 2] = (i + 1) * length + j + 1;
				count += 3;
			}
			else if (0 == burntSituation) {
				glm::vec3 fireOrigin;
				fireOrigin = (cloth[i][j + 1].position + cloth[i + 1][j].position + cloth[i + 1][j + 1].position) / 3.0f;
				fireBirthplace.push_back(fireOrigin);
			}
		}
	}

	//#pragma omp parallel for
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < length; j++) {
			triangleCloth[5 * (i * length + j)] = cloth[i][j].position.x;
			triangleCloth[5 * (i * length + j) + 1] = cloth[i][j].position.y;
			triangleCloth[5 * (i * length + j) + 2] = cloth[i][j].position.z;
			triangleCloth[5 * (i * length + j) + 3] = cloth[i][j].textureCoordinate.x;
			triangleCloth[5 * (i * length + j) + 4] = cloth[i][j].textureCoordinate.y;
		}
	}

	glBindVertexArray(id.vao);

	glBindBuffer(GL_ARRAY_BUFFER, id.bufferID_vertex);
	glBufferData(GL_ARRAY_BUFFER, length * width * 5 * sizeof(float), triangleCloth, GL_STREAM_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id.bufferID_indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (length - 1) * (width - 1) * 2 * 3 * sizeof(unsigned int), indices, GL_STREAM_DRAW);

	int posAttrib = glGetAttribLocation(shader, "position");
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glEnableVertexAttribArray(posAttrib);

	int colAttrib = glGetAttribLocation(shader, "inTexcoord");
	glVertexAttribPointer(colAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(colAttrib);

	glBindVertexArray(0);

	delete indices;
	delete triangleCloth;
	return fireBirthplace;
}

int importTexture() {
	
	SDL_Surface* surface = SDL_LoadBMP("models/elf.bmp");
	if (surface == NULL) { //If it failed, print the error
		printf("Error: \"%s\"\n", SDL_GetError());
	}

	GLuint tex;
	glGenTextures(1, &tex);

	//Load the texture into memory
	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	//What to do outside 0-1 range
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, surface->w, surface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);

	SDL_FreeSurface(surface);

	return tex;
}

void displayCloth(int shaderProgram, bufferID id, GLuint length, GLuint width) {
	
		glBindVertexArray(id.vao);
		glBindBuffer(GL_ARRAY_BUFFER, id.bufferID_vertex);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id.bufferID_indices);

		glUniform1i(glGetUniformLocation(shaderProgram, "texID"), id.tex); //Set texture ID to use
		
		glDrawElements(GL_TRIANGLES, (length - 1) * (width - 1) * 2 * 3, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	
}
