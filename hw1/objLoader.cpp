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
#include "stdafx.h"
#include "objLoader.h"
#include <stdio.h>
using namespace std;

verticeAndIndices* objLoader(char* path, char* path2, char* path3) {
	
	FILE* objFile = fopen(path, "r");
	FILE* mtlFile = fopen(path2, "r");
	
	if (objFile == 0) {
		cout << "Could not open '.obj' file" << endl;
	}
	else if (mtlFile == 0) {
		cout << "Could not open '.mtl' file" << endl;
	}

	int num_objects = 0;
	int num_vertices = 0, num_normals = 0, num_texture = 0;
	while (1) {// first scan to get the numbers of vertices, triangles, normals and texture coordinates.
		char lineHeader[100];
		int res = fscanf(objFile, "%s", lineHeader);
		if (res == EOF) {// end of file
			break;
		}
		if (strcmp(lineHeader, "v") == 0) {// vertices
			num_vertices++;
		}
		else if (strcmp(lineHeader, "vt") == 0) {// texture
			num_texture++;
		}
		else if (strcmp(lineHeader, "vn") == 0) {// normals
			num_normals++;
		}
		else if (strcmp(lineHeader, "o") == 0) {// mark of object
			num_objects++;
		}
	}
	float* vertices = new float[num_vertices * 3];
	float* texture = new float[num_texture * 2];
	float* normals = new float[num_normals * 3];

	num_vertices = 0, num_normals = 0, num_texture = 0;
	rewind(objFile);

	while (1) {// second scan to get real data of vertices, normals and texture coordinates.
		char lineHeader[50];
		int res = fscanf(objFile, "%s", lineHeader);
		if (res == EOF) {
			break;
		}
		if (strcmp(lineHeader, "v") == 0) {// vertices
			fscanf(objFile, "%f %f %f\n", &vertices[num_vertices], &vertices[num_vertices + 1], &vertices[num_vertices + 2]);
			/*vertices[num_vertices] = vertices[num_vertices] / 30.0;
			vertices[num_vertices + 1] = vertices[num_vertices + 1] / 30.0;
			vertices[num_vertices + 2] = vertices[num_vertices + 2] / 30.0;*/
			num_vertices = num_vertices + 3;
		}
		else if (strcmp(lineHeader, "vt") == 0) {// texture
			fscanf(objFile, "%f %f\n", &texture[num_texture], &texture[num_texture + 1]);
			texture[num_texture + 1] = 1.0f - texture[num_texture + 1];
			num_texture = num_texture + 2;
		}
		else if (strcmp(lineHeader, "vn") == 0) {// normals
			fscanf(objFile, "%f %f %f\n", &normals[num_normals], &normals[num_normals + 1], &normals[num_normals + 2]);
			num_normals = num_normals + 3;
		}
	}

	verticeAndIndices *vai = new verticeAndIndices[num_objects];
	rewind(objFile);

	for (int object = 0; object < num_objects; object++) {
		int num_vertices_index = 0, num_texture_index = 0, num_normals_index = 0;
		int navigation = 0;
		while (1) {// first scan to get the numbers of triangles of a certain object of the model.
			char lineHeader[100];
			int res = fscanf(objFile, "%s", lineHeader);
			if (res == EOF || (object + 2 == navigation)) {
				break;
			}
			if (strcmp(lineHeader, "f") == 0 && (object + 1) == navigation) {// surface
				num_vertices_index++;
			}
			else if (strcmp(lineHeader, "usemtl") == 0 && (object + 1) == navigation) {// get the material of this object
				fscanf(objFile, "%s\n", &lineHeader);
				while (1) {
					char findMaterial[50];
					int check = fscanf(mtlFile, "%s", findMaterial);
					if (check == EOF) {
						cout << "Cannot find the material." << endl;
						break;
					}
					if (strcmp(findMaterial, lineHeader) == 0) {// find the material in the .mtl file
						for (int material = 0; material < 8; material++) {
							fscanf(mtlFile, "%s", findMaterial);
							cout << findMaterial << endl;
							if (strcmp(findMaterial, "Ns") == 0) {
								fscanf(mtlFile, "%f\n", &vai[object].mtl.ns);
							}
							else if (strcmp(findMaterial, "Ka") == 0) {
								fscanf(mtlFile, "%f %f %f\n", &vai[object].mtl.ar, &vai[object].mtl.ag, &vai[object].mtl.ab);
							}
							else if (strcmp(findMaterial, "Kd") == 0) {
								fscanf(mtlFile, "%f %f %f\n", &vai[object].mtl.dr, &vai[object].mtl.dg, &vai[object].mtl.db);
							}
							else if (strcmp(findMaterial, "Ks") == 0) {
								fscanf(mtlFile, "%f %f %f\n", &vai[object].mtl.sr, &vai[object].mtl.sg, &vai[object].mtl.sb);
							}
							else if (strcmp(findMaterial, "Ni") == 0) {
								fscanf(mtlFile, "%f\n", &vai[object].mtl.Ni);
							}
							else if (strcmp(findMaterial, "d") == 0) {
								fscanf(mtlFile, "%f\n", &vai[object].mtl.d);
							}
							else if (strcmp(findMaterial, "illum") == 0) {
								fscanf(mtlFile, "%f\n", &vai[object].mtl.illum);
							}
							else if (strcmp(findMaterial, "map_Kd") == 0) {
								fscanf(mtlFile, "%s\n", &vai[object].mtl.texturePath);
								cout << vai[object].mtl.texturePath;
								char newpath[100];
								strcpy_s(newpath, path3);
								strcat_s(newpath, vai[object].mtl.texturePath);
								strcpy_s(vai[object].mtl.texturePath, newpath);
								break;
							}
						}
						//cout << vai[object].mtl.texturePath << endl;
						rewind(mtlFile);
						//cout << vai[object].mtl.dr << ' ' << vai[object].mtl.dg << ' ' << vai[object].mtl.db << ' ' << endl;
						break;
					}
				}
			}
			else if (strcmp(lineHeader, "o") == 0) {
				navigation++;
			}
		}

		int* vertices_index = new int[num_vertices_index * 3];
		int* texture_index = new int[num_vertices_index * 3];
		int* normals_index = new int[num_vertices_index * 3];
		num_vertices_index = 0, num_texture_index = 0, num_normals_index = 0;
		rewind(objFile);

		navigation = 0;
		while (1) {// second scan to get the indices of vertices, normals and texture coordinates from surfaces.
			char lineHeader[50];
			int res = fscanf(objFile, "%s", lineHeader);
			if (res == EOF || (object + 2 == navigation)) {
				break;
			}
			if (strcmp(lineHeader, "f") == 0 && (object + 1) == navigation) {// surface
				int vertexIndex[3], uvIndex[3], normalIndex[3];
				fscanf(objFile, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
				vertices_index[num_vertices_index++] = vertexIndex[0];
				vertices_index[num_vertices_index++] = vertexIndex[1];
				vertices_index[num_vertices_index++] = vertexIndex[2];
				texture_index[num_texture_index++] = uvIndex[0];
				texture_index[num_texture_index++] = uvIndex[1];
				texture_index[num_texture_index++] = uvIndex[2];
				normals_index[num_normals_index++] = normalIndex[0];
				normals_index[num_normals_index++] = normalIndex[1];
				normals_index[num_normals_index++] = normalIndex[2];
			}
			else if (strcmp(lineHeader, "o") == 0) {
				navigation++;
			}
		}

		GLuint* rearranged_index = new GLuint[num_vertices_index];
		GLfloat* rearranged_vertices = new GLfloat[num_vertices_index * 8];
		int added_vertices = 0;
		for (int i = 0; i < num_normals_index; i++) {// rearrange storage of both vertices and indices.
			if (i != 0) {
				bool already_have = FALSE;
				for (int j = 0; j < added_vertices * 8; j = j + 8) {// See whether the vertices(position + texture + normal) are already in the rearranged array
					if (rearranged_vertices[j] == vertices[vertices_index[i] * 3 - 3] && rearranged_vertices[j + 1] == vertices[vertices_index[i] * 3 - 2] && rearranged_vertices[j + 2] == vertices[vertices_index[i] * 3 - 1] &&
						rearranged_vertices[j + 3] == texture[texture_index[i] * 2 - 2] && rearranged_vertices[j + 4] == texture[texture_index[i] * 2 - 1] &&
						rearranged_vertices[j + 5] == normals[normals_index[i] * 3 - 3] && rearranged_vertices[j + 6] == normals[normals_index[i] * 3 - 2] && rearranged_vertices[j + 7] == normals[normals_index[i] * 3 - 1]) {
						already_have = TRUE;
						rearranged_index[i] = j / 8;
						break;
					}
				}
				if (!already_have) {// put position, texture and normals in order in one array
					rearranged_vertices[added_vertices * 8] = vertices[vertices_index[i] * 3 - 3];
					rearranged_vertices[added_vertices * 8 + 1] = vertices[vertices_index[i] * 3 - 2];
					rearranged_vertices[added_vertices * 8 + 2] = vertices[vertices_index[i] * 3 - 1];
					rearranged_vertices[added_vertices * 8 + 3] = texture[texture_index[i] * 2 - 2];
					rearranged_vertices[added_vertices * 8 + 4] = texture[texture_index[i] * 2 - 1];
					rearranged_vertices[added_vertices * 8 + 5] = normals[normals_index[i] * 3 - 3];
					rearranged_vertices[added_vertices * 8 + 6] = normals[normals_index[i] * 3 - 2];
					rearranged_vertices[added_vertices * 8 + 7] = normals[normals_index[i] * 3 - 1];
					rearranged_index[i] = added_vertices;
					added_vertices++;
				}
			}
			else {
				rearranged_vertices[added_vertices * 8] = vertices[vertices_index[i] * 3 - 3];
				rearranged_vertices[added_vertices * 8 + 1] = vertices[vertices_index[i] * 3 - 2];
				rearranged_vertices[added_vertices * 8 + 2] = vertices[vertices_index[i] * 3 - 1];
				rearranged_vertices[added_vertices * 8 + 3] = texture[texture_index[i] * 2 - 2];
				rearranged_vertices[added_vertices * 8 + 4] = texture[texture_index[i] * 2 - 1];
				rearranged_vertices[added_vertices * 8 + 5] = normals[normals_index[i] * 3 - 3];
				rearranged_vertices[added_vertices * 8 + 6] = normals[normals_index[i] * 3 - 2];
				rearranged_vertices[added_vertices * 8 + 7] = normals[normals_index[i] * 3 - 1];
				added_vertices++;
				rearranged_index[0] = 0;
			}
			
		}

		/*cout << endl;
		for (int n = 0; n < added_vertices; n++) {
			for (int m = 0; m < 8; m++) {
				cout << rearranged_vertices[n * 8 + m] << ' ';
			}
			cout << rearranged_index[n] <<endl;
		}*/

		delete vertices_index;
		delete texture_index;
		delete normals_index;

		GLfloat* r_vertices = new GLfloat[added_vertices * 8];
		for (int i = 0; i < added_vertices * 8; i = i + 8) {
			for (int j = 0; j < 8; j++) {
				r_vertices[i + j] = rearranged_vertices[i + j];
				//cout << r_vertices[i + j]<<' ';
			}
			//cout << endl;
		}
		/*for (int i = 0; i < num_normals_index; i = i + 3) {
		for (int j = 0; j < 3; j++) {
		cout << rearranged_index[i + j] << ' ';
		}
		cout << endl;
		}*/

		vai[object].vertices = r_vertices;
		vai[object].indices = rearranged_index;
		vai[object].num_vertices = added_vertices;
		vai[object].num_indices = num_normals_index;
		vai[object].num_objects = num_objects;
		rewind(objFile);
	}

	delete vertices;
	delete texture;
	delete normals;

	fclose(objFile);
	return vai;
}

bufferID* newBufferForModel(verticeAndIndices* model, GLuint programID) {
	bufferID* vaoBuffer = new bufferID[model[0].num_objects];
	for (int i = 0; i < model[0].num_objects; i++) {
		glGenVertexArrays(1, &vaoBuffer[i].vao); //Create a VAO
		glBindVertexArray(vaoBuffer[i].vao); //Bind the above created VAO to the current context
		
		glGenBuffers(1, &vaoBuffer[i].bufferID_vertex);
		glBindBuffer(GL_ARRAY_BUFFER, vaoBuffer[i].bufferID_vertex);
		glBufferData(GL_ARRAY_BUFFER, model[i].num_vertices * 8 * sizeof(float), model[i].vertices, GL_STATIC_DRAW);

		glGenBuffers(1, &vaoBuffer[i].bufferID_indices);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vaoBuffer[i].bufferID_indices);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, model[i].num_indices * sizeof(unsigned int), model[i].indices, GL_STATIC_DRAW);

		int posAttrib = glGetAttribLocation(programID, "position");
		glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
		glEnableVertexAttribArray(posAttrib);

		int texAttrib = glGetAttribLocation(programID, "inTexcoord");
		glEnableVertexAttribArray(texAttrib);
		glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

		int normAttrib = glGetAttribLocation(programID, "inNormal");
		glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
		glEnableVertexAttribArray(normAttrib);

		glBindVertexArray(0);
	}
	return vaoBuffer;
}

void generateTexture(verticeAndIndices* model) {
	for (int i = 0; i < model[0].num_objects; i++) {
		if (strcmp(model[i].mtl.texturePath, "No") == 0) {
			continue;
		}

		SDL_Surface* surface = SDL_LoadBMP(model[i].mtl.texturePath);
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

		model[i].tex = tex;
	}
}

void display(int shaderProgram, verticeAndIndices* model, bufferID* modelBuffer, glm::mat4 transform) {
	for (int object = 0; object < model[0].num_objects; object++) {
		glm::mat4 ori_model;
		glBindVertexArray(modelBuffer[object].vao);
		glBindBuffer(GL_ARRAY_BUFFER, modelBuffer[object].bufferID_vertex);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelBuffer[object].bufferID_indices);

		
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(transform));

		glUniform1f(glGetUniformLocation(shaderProgram, "dr"), model[object].mtl.dr);
		glUniform1f(glGetUniformLocation(shaderProgram, "dg"), model[object].mtl.dg);
		glUniform1f(glGetUniformLocation(shaderProgram, "db"), model[object].mtl.db);
		glUniform1f(glGetUniformLocation(shaderProgram, "ar"), model[object].mtl.ar);
		glUniform1f(glGetUniformLocation(shaderProgram, "ag"), model[object].mtl.ag);
		glUniform1f(glGetUniformLocation(shaderProgram, "ab"), model[object].mtl.ab);
		glUniform1f(glGetUniformLocation(shaderProgram, "sr"), model[object].mtl.sr);
		glUniform1f(glGetUniformLocation(shaderProgram, "sg"), model[object].mtl.sg);
		glUniform1f(glGetUniformLocation(shaderProgram, "sb"), model[object].mtl.sb);
		glUniform1f(glGetUniformLocation(shaderProgram, "ns"), model[object].mtl.ns);

		if (strcmp(model[object].mtl.texturePath, "No") == 0) {
			glUniform1i(glGetUniformLocation(shaderProgram, "texID"), 0); //Set texture ID to use
		}
		else {
			//cout << model[object].tex << endl;
			glActiveTexture(GL_TEXTURE0 + model[object].tex);
			glBindTexture(GL_TEXTURE_2D, model[object].tex);
			char temp[10];
			strcpy(temp, "tex[");
			char temp2[5];
			_itoa(model[object].tex, temp2, 10);
			strcat(temp, temp2);
			strcat(temp, "]");
			glUniform1i(glGetUniformLocation(shaderProgram, temp), model[object].tex);
			glUniform1i(glGetUniformLocation(shaderProgram, "texID"), model[object].tex); //Set texture ID to use
		}
		glDrawElements(GL_TRIANGLES, model[object].num_indices, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
}
