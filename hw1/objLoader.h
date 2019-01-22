#pragma once
#include "glad\glad.h"
#include "include\SDL.h"
#include "include\SDL_opengl.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <vector>
using namespace std;

struct material {
	GLfloat ar, ag, ab, dr, dg, db, sr, sg, sb, ns, Ni, d;
	GLuint illum;
	char texturePath[50] = "No";
};

struct verticeAndIndices {
	GLfloat* vertices;
	GLuint* indices;
	int num_vertices;
	int num_indices;
	int num_objects;
	material mtl;
	GLuint tex;
};

struct bufferID {
	GLuint vao, bufferID_vertex, bufferID_indices, tex;
};

void drawGeometry(int shaderProgram, GLfloat dt, vector<glm::vec3> path, vector<glm::vec3> path2);

verticeAndIndices* objLoader(char* path, char* path2, char* path3);

bufferID* newBufferForModel(verticeAndIndices* model, GLuint programID);

void generateTexture(verticeAndIndices* model);

void display(int shaderProgram, verticeAndIndices* model, bufferID* modelBuffer, glm::mat4 transform);

