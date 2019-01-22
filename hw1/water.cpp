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

water** shallowWater(water** sw, GLfloat dt, GLfloat dx, GLuint nx) {
	dt /= 5.0f;
	/*for (int i = 1; i < (nx - 1); i++) {
		for (int j = 1; j < (nx - 1); j++) {
			sw[i][j].position.y -= dt * ((sw[i + 1][j].uh + sw[i][j + 1].uh + sw[i][j - 1].uh + sw[i - 1][j].uh) / 4.0f - sw[i][j].uh) / dx;
			sw[i][j].uh -= dt * (0.1f * sw[i][j].uh 
				+ (pow(sw[i + 1][j].uh, 2) / sw[i + 1][j].position.y
				+ pow(sw[i - 1][j].uh, 2) / sw[i - 1][j].position.y
				+ pow(sw[i][j + 1].uh, 2) / sw[i][j + 1].position.y
				+ pow(sw[i][j - 1].uh, 2) / sw[i][j - 1].position.y) / 4.0f
				- pow(sw[i][j].uh, 2) / sw[i][j].position.y) / dx;
			sw[i][j].uh -= 0.5f * 9.8f * dt * ((pow(sw[i + 1][j].position.y, 2)
				+ pow(sw[i - 1][j].position.y, 2)
				+ pow(sw[i][j + 1].position.y, 2)
				+ pow(sw[i][j - 1].position.y, 2)) / 4.0f
				- pow(sw[i][j].position.y, 2)) / dx;
		}
	}*/
	for (int i = 1; i < (nx - 1); i++) {
		for (int j = 1; j < (nx - 1); j++) {
			//horizontal
			sw[i][j].position.y -= dt * (sw[i][j + 1].uh - sw[i][j].uh) / dx;
			sw[i][j + 1].uh -= dt * (0.2f * sw[i][j + 1].uh
				+ pow(sw[i][j + 1].uh, 2) / sw[i][j + 1].position.y
				- pow(sw[i][j].uh, 2) / sw[i][j].position.y) / dx;
			sw[i][j + 1].uh -= 0.5f * 9.8f * dt * (pow(sw[i][j + 1].position.y, 2)
				- pow(sw[i][j].position.y, 2)) / dx;

			/*sw[i][j].position.y -= dt * (sw[i][j - 1].uh - sw[i][j].uh) / dx;
			sw[i][j - 1].uh -= dt * (0.02f * sw[i][j - 1].uh
				+ pow(sw[i][j - 1].uh, 2) / sw[i][j - 1].position.y
				- pow(sw[i][j].uh, 2) / sw[i][j].position.y) / dx;
			sw[i][j - 1].uh -= 0.5f * 9.8f * dt * (pow(sw[i][j - 1].position.y, 2)
				- pow(sw[i][j].position.y, 2)) / dx;*/

			//vertical
			sw[i][j].position.y -= dt * (sw[i + 1][j].uh - sw[i][j].uh) / dx;
			sw[i + 1][j].uh -= dt * (0.2f * sw[i + 1][j].uh
				+ pow(sw[i + 1][j].uh, 2) / sw[i + 1][j].position.y
				- pow(sw[i][j].uh, 2) / sw[i][j].position.y) / dx;
			sw[i + 1][j].uh -= 0.5f * 9.8f * dt * (pow(sw[i + 1][j].position.y, 2)
				- pow(sw[i][j].position.y, 2)) / dx;

			/*sw[i][j].position.y -= dt * (sw[i - 1][j].uh - sw[i][j].uh) / dx;
			sw[i - 1][j].uh -= dt * (0.02f * sw[i - 1][j].uh
				+ pow(sw[i - 1][j].uh, 2) / sw[i - 1][j].position.y
				- pow(sw[i][j].uh, 2) / sw[i][j].position.y) / dx;
			sw[i - 1][j].uh -= 0.5f * 9.8f * dt * (pow(sw[i - 1][j].position.y, 2)
				- pow(sw[i][j].position.y, 2)) / dx;*/

			glm::vec3 a, b;
			a = sw[i][j - 1].position - sw[i + 1][j].position;
			//b = sw[i][j + 1].position - sw[i - 1][j].position;
			sw[i][j].normal = a;
			sw[i][j].normal /= sqrt(glm::dot(sw[i][j].normal, sw[i][j].normal));
		}
	}
	return sw;
}

void sendWaterToBuffer(water** sw, GLuint nx, bufferID waterBufferID, GLuint shader) {
	GLfloat* surface = new GLfloat[9 * nx * nx];
	for (int i = 0; i < nx; i++) {
		for (int j = 0; j < nx; j++) {
			surface[9 * (i * nx + j)] = sw[i][j].position.x;
			surface[9 * (i * nx + j) + 1] = sw[i][j].position.y;
			surface[9 * (i * nx + j) + 2] = sw[i][j].position.z;
			surface[9 * (i * nx + j) + 3] = sw[i][j].color.r;
			surface[9 * (i * nx + j) + 4] = sw[i][j].color.g;
			surface[9 * (i * nx + j) + 5] = sw[i][j].color.b;
			surface[9 * (i * nx + j) + 6] = sw[i][j].normal.x;
			surface[9 * (i * nx + j) + 7] = sw[i][j].normal.y;
			surface[9 * (i * nx + j) + 8] = sw[i][j].normal.z;
		}		
	}

	GLuint* indices = new GLuint[(nx - 1) * (nx - 1) * 2 * 3];
	GLuint count = 0;
	for (GLuint i = 0; i < (nx - 1); i++) {
		for (GLuint j = 0; j < (nx - 1); j++) {
			indices[count] = i * nx + j;
			indices[count + 1] = i * nx + j + 1;
			indices[count + 2] = (i + 1) * nx + j;
			count += 3;
			indices[count] = i * nx + j + 1;
			indices[count + 1] = (i + 1) * nx + j;
			indices[count + 2] = (i + 1) * nx + j + 1;
			count += 3;
		}
	}

	glBindVertexArray(waterBufferID.vao);

	glBindBuffer(GL_ARRAY_BUFFER, waterBufferID.bufferID_vertex);
	glBufferData(GL_ARRAY_BUFFER, 9 * nx * nx * sizeof(float), surface, GL_STREAM_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waterBufferID.bufferID_indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (nx - 1) * (nx - 1) * 2 * 3 * sizeof(unsigned int), indices, GL_STREAM_DRAW);

	int posAttrib = glGetAttribLocation(shader, "position");
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), 0);
	glEnableVertexAttribArray(posAttrib);

	int colAttrib = glGetAttribLocation(shader, "inColor");
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(colAttrib);

	int normAttrib = glGetAttribLocation(shader, "inNormal");
	glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(normAttrib);

	glBindVertexArray(0);
	delete surface;
	delete indices;
}

void displayWater(GLuint nx, bufferID waterBufferID, GLuint shader) {
	glPointSize(3.0f);
	glBindVertexArray(waterBufferID.vao);
	glBindBuffer(GL_ARRAY_BUFFER, waterBufferID.bufferID_vertex);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waterBufferID.bufferID_indices);
	//glDrawArrays(GL_POINTS, 0, nx * nx);
	glDrawElements(GL_TRIANGLES, (nx - 1) * (nx - 1) * 2 * 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}