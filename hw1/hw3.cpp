// hw1.cpp : Defines the entry point for the console application.
//
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
#include <mmsystem.h>
#include <vector>
#include "objLoader.h"
#include "particleSystem.h"
#include "cloth.h"
#include "water.h"
#include "PRM.h"
using namespace std;

int screenWidth = 800;
int screenHeight = 800;
bool saveOutput = false;
float lastTime = 0;
float viewRotate = 0.01;
//SJG: Store the object coordinates
//You should have a representation for the state of each object

//glm::vec3 position(0.0f, 7.0f, 12.0f);  //Cam Position
//glm::vec3 position(0.0f, 12.0f, 3.0f);  //Cam Position
glm::vec3 position(0.0f, 12.0f, 4.0f);  //Cam Position
//glm::vec3 position(-7.0f, 3.0f, -6.0f);  //Cam Position
//glm::vec3 center(-1.0f, 1.0f, 0);  //Look at point
glm::vec3 center(0, 0, 0);  //Look at point

bool DEBUG_ON = true;
GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName);
bool fullscreen = false;

int boidNumber = 26;
boid* boids;
verticeAndIndices** boidModel;
verticeAndIndices* Floor;
verticeAndIndices* football;
verticeAndIndices* queen;
verticeAndIndices* tree;

bufferID circleBeneathModelBuffer;
bufferID** boidBuffer;
bufferID* floorBuffer;
bufferID* footballBuffer;
bufferID* queenBuffer;
bufferID* treeBuffer;
glm::vec3 floorPosition(0, 0, 0);
glm::vec3 floorVelocity(0, 0, 0);
glm::vec3 footballPosition(0, 0, 0);
glm::vec3 footballVelocity(0, 0, 0);
glm::vec3 treePosition(0, 0, 0);
GLfloat footballRadius;
GLfloat floorWidth;

GLuint length = 30;
GLuint width = 30;
GLfloat ks = 3000;
GLfloat kd = 30;
glm::vec3 windSpeed(7.0f, 0, 0);
GLfloat burntRadius = 0; 
glm::vec2 burntCenter(width - 1, length - 1);
glm::vec3 fireColor(0.92f, 1.0f, 0.2f);
GLfloat fireLifespan = 1.0f;
GLuint generationRate = 200;
GLfloat separationRadius = 0.4f;
GLfloat alignmentRadius = 0.8f;
GLfloat cohesionRadius = 0.6f;
GLfloat boidRouteRadius = 4.0f;
GLfloat boidRouteAngle = 0;

GLuint nx = 30;
GLfloat dx = 0.2f;

GLuint sampleNumber = 15;
GLfloat cSpaceLength = 9.0f;
GLfloat cSpaceWidth = 9.0f;
glm::vec3 startingPoint(-3.5f, 0, 3.5f);
glm::vec3 terminalPoint(3.5f, 0, -3.5f);
vector<stateOnRoute> queenState(4);
vector<stateOnRoute> treeState;
circularObstacle circle;
GLfloat queenRadius;
GLfloat treeRadius;
bool obstacleAdded = false;

float boidsTriangle[] = {-0.25f, -0.25f, -0.25f, 0.9f, 0.7f, 0.2f, -0.577f, -0.577f, -0.577f,
0.25f, -0.25f, -0.25f, 0.7f, 0.9f, 0.2f, 0.577f, -0.577f , -0.577f,
-0.25f, -0.25f, 0.25f, 0.2f, 0.7f, 0.9f, -0.577f, -0.577f , 0.577f,
0.25f, -0.25f, 0.25f, 0.2f, 0.9f, 0.7f, 0.577f, -0.577f , 0.577f,
0, 0.5f, 0, 0.9f, 0.2f, 0.7f, 0, 1.0f, 0};

GLuint boidsIndex[] = { 0, 1, 4,
0, 4, 2,
2, 4 ,3,
4, 3, 1,
0, 2, 3,
0, 1, 3 };

GLuint circleIndex[] = { 0, 1, 2,
0, 2, 3,
0, 3, 4,
0, 4 ,5,
0, 5, 6,
0, 6, 7,
0, 7, 8,
0, 8, 9,
0, 9, 10,
0, 10, 11,
0, 11, 12,
0, 12, 1};

int main(int argc, char *argv[]) {
	SDL_Init(SDL_INIT_VIDEO);  //Initialize Graphics (for OpenGL)
							   //Ask SDL to get a recent version of OpenGL (3 or greater)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	//Create a window (offsetx, offsety, width, height, flags)
	SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 100, 100, screenWidth, screenHeight, SDL_WINDOW_OPENGL);

	//Create a context to draw in
	SDL_GLContext context = SDL_GL_CreateContext(window);

	//Load OpenGL extentions with GLAD
	if (gladLoadGLLoader(SDL_GL_GetProcAddress)) {
		printf("\nOpenGL loaded\n");
		printf("Vendor:   %s\n", glGetString(GL_VENDOR));
		printf("Renderer: %s\n", glGetString(GL_RENDERER));
		printf("Version:  %s\n\n", glGetString(GL_VERSION));
	}
	else {
		printf("ERROR: Failed to initialize OpenGL context.\n");
		return -1;
	}

	// Load Models
	Floor = objLoader("models/floor/floor.obj", "models/floor/floor.mtl", "models/floor/");
	football = objLoader("models/football/football.obj", "models/football/football.mtl", "models/football/");
	queen = objLoader("models/queen/queen.obj", "models/queen/queen.mtl", "models/queen/");
	tree = objLoader("models/tree/tree.obj", "models/tree/tree.mtl", "models/tree/");
	boids = new boid[boidNumber];
	boidModel = new verticeAndIndices*[boidNumber];
	for (int i = 0; i < boidNumber; i++) {
		boidModel[i] = football;
		GLfloat offsetx, offsety, offsetz;
		offsetx = (rand() % 40) / 10.0f - 2.0f;
		offsety = (rand() % 40) / 10.0f - 2.0f;
		offsetz = (rand() % 40) / 10.0f - 2.0f;
		boids[i].position = footballPosition + glm::vec3(offsetx, offsety, offsetz);
		boids[i].velocity = glm::vec3(offsety, offsetz, offsetz);
		boids[i].maxSpeed = 1.0f;
		boids[i].maxForce = 0.3f;
		boids[i].label = i;
		boids[i].acceleration = glm::vec3(0, 0, 0);
		glGenVertexArrays(1, &boids[i].buffer.vao);
		glGenBuffers(1, &boids[i].buffer.bufferID_vertex);
		glGenBuffers(1, &boids[i].buffer.bufferID_indices);
	}
	
	glGenVertexArrays(1, &circleBeneathModelBuffer.vao);
	glGenBuffers(1, &circleBeneathModelBuffer.bufferID_vertex);
	glGenBuffers(1, &circleBeneathModelBuffer.bufferID_indices);

	// Compute the size of tree
	GLfloat max = -999, min = 999;
	for (int i = 0; i < tree->num_vertices; i++) {
		if (abs(tree->vertices[i * 8]) > max) {
			max = abs(tree->vertices[i * 8]);
		}
		if (abs(tree->vertices[i * 8 + 2]) > max) {
			max = abs(tree->vertices[i * 8 + 2]);
		}
	}
	treeRadius = max;

	// Basic Configuration of trees
	/*for (int i = 0; i < 3; i++) {
	stateOnRoute trees;
	trees.position = glm::vec3(-1.8f, 0, 0) + glm::vec3(1.8f * i, 0, 0);
	trees.velocity = glm::vec3(0, 0, 0);
	trees.radius = treeRadius / 10000.0f;
	treeState.push_back(trees);
	}*/

	// Configuration of trees for global navigation
	for (int i = 0; i < 5; i++) {
		stateOnRoute trees;
		trees.position = glm::vec3(-2.0f, 0, 0) + glm::vec3(1.0f * i, 0, 0);
		trees.velocity = glm::vec3(0, 0, 0);
		trees.radius = treeRadius / 10000.0f;
		treeState.push_back(trees);
	}
	for (int i = 0; i < 5; i++) {
		if (i != 2) {
			stateOnRoute trees;
			trees.position = glm::vec3(0, 0, -2.0f) + glm::vec3(0, 0, 1.0f * i);
			trees.velocity = glm::vec3(0, 0, 0);
			trees.radius = treeRadius / 10000.0f;
			treeState.push_back(trees);
		}
	}

	// Compute the size of the queen
	max = -999, min = 999;
	for (int i = 0; i < queen->num_vertices; i++) {
		if (abs(queen->vertices[i * 8]) > max) {
			max = abs(queen->vertices[i * 8]);
		}
		if (abs(queen->vertices[i * 8 + 2]) > max) {
			max = abs(queen->vertices[i * 8 + 2]);
		}
	}
	queenRadius = max;

	queenState[0].agentDistance = 0;
	queenState[0].agentSpeed = 0.7f;
	queenState[0].position = startingPoint;
	queenState[0].goal = terminalPoint;
	queenState[0].velocity = glm::normalize(queenState[0].goal - queenState[0].position) * 0.7f;
	queenState[0].radius = queenRadius / 5.0f;
	queenState[0].position /= 3.0f;
	queenState[0].PRM = PRM(queenState[0], treeState, sampleNumber);

	queenState[1].agentDistance = 0;
	queenState[1].agentSpeed = 0.7f;
	queenState[1].position = terminalPoint;
	queenState[1].goal = startingPoint;
	queenState[1].velocity = glm::normalize(queenState[1].goal - queenState[1].position) * 0.7f;
	queenState[1].radius = queenRadius / 5.0f;
	queenState[1].position /= 3.0f;
	queenState[1].PRM = PRM(queenState[1], treeState, sampleNumber);

	queenState[2].agentDistance = 0;
	queenState[2].agentSpeed = 0.7f;
	queenState[2].position = terminalPoint + glm::vec3(-7.0f, 0, 0);
	queenState[2].goal = startingPoint + glm::vec3(7.0f, 0, 0);
	queenState[2].velocity = glm::normalize(queenState[1].goal - queenState[1].position) * 0.7f;
	queenState[2].radius = queenRadius / 5.0f;
	queenState[2].position /= 3.0f;
	queenState[2].PRM = PRM(queenState[2], treeState, sampleNumber);

	queenState[3].agentDistance = 0;
	queenState[3].agentSpeed = 0.7f;
	queenState[3].position = startingPoint + glm::vec3(7.0f, 0, 0);
	queenState[3].goal = terminalPoint + glm::vec3(-7.0f, 0, 0);
	queenState[3].velocity = glm::normalize(queenState[1].goal - queenState[1].position) * 0.7f;
	queenState[3].radius = queenRadius / 5.0f;
	queenState[3].position /= 3.0f;
	queenState[3].PRM = PRM(queenState[3], treeState, sampleNumber);

	// Compute the size of the football
	for (int i = 0; i < football->num_vertices; i++) {
		if (football->vertices[i * 8 + 1] > max) {
			max = football->vertices[i * 8 + 1];
		}
		if (football->vertices[i * 8 + 1] < min) {
			min = football->vertices[i * 8 + 1];
		}
	}
	footballRadius = (max - min) / 2.0f;

	// Compute the size of the floor
	max = -999;
	min = 999;
	for (int i = 0; i < Floor->num_vertices; i++) {
		if (Floor->vertices[i * 8] > max) {
			max = Floor->vertices[i * 8];
		}
		if (Floor->vertices[i * 8] < min) {
			min = Floor->vertices[i * 8];
		}
	}
	floorWidth = max - min;

	int texturedShader = InitShader("vertexTex.glsl", "fragmentTex.glsl");
	int clothShader = InitShader("clothVertex.glsl", "clothFragment.glsl");
	int phongShader = InitShader("phongVertex.glsl", "phongFragment.glsl");
	int particleShader = InitShader("particleVertex.glsl", "particleFragment.glsl");

	// Create buffer for models
	floorBuffer = newBufferForModel(Floor, texturedShader);
	footballBuffer = newBufferForModel(football, texturedShader);
	queenBuffer = newBufferForModel(queen, texturedShader);
	treeBuffer = newBufferForModel(tree, texturedShader);
	boidBuffer = new bufferID*[boidNumber];
	for (int i = 0; i < boidNumber; i++) {
		boidBuffer[i] = newBufferForModel(boidModel[i], texturedShader);
	}

	// Load texture for models
	generateTexture(Floor);
	generateTexture(football);
	generateTexture(queen);
	generateTexture(tree);
	for (int i = 0; i < boidNumber; i++) {
		generateTexture(boidModel[i]);
	}

	// Generate cloth.
	node** clo = new node*[width];
	for (int i = 0; i < width; i++) {
		clo[i] = new node[length];
	}
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < length; j++) {
			clo[i][j].position = glm::vec3(4.0f / length * (j + 1) - 2.0f, 3.0f, 4.0f / width * (i + 1) - 2.0f);
			clo[i][j].verticalInterval = 4.0f / (length - 1) * 0.95f;
			clo[i][j].horizontalInterval = 4.0f / (width - 1);
			clo[i][j].textureCoordinate = glm::vec2(i / (width - 1.0f), j / (length - 1.0f));
			clo[i][j].velocity = glm::vec3(0, 0, 0);
			clo[i][j].kd = kd;
			clo[i][j].ks = ks;
		}
		//cout << endl;
	}
	bufferID clothBufferID;
	glGenVertexArrays(1, &clothBufferID.vao);
	glGenBuffers(1, &clothBufferID.bufferID_vertex);
	glGenBuffers(1, &clothBufferID.bufferID_indices);
	clothBufferID.tex = importTexture();

	// Generate shallow water.
	water** sw = new water*[nx];
	for (int i = 0; i < nx; i++) {
		sw[i] = new water[nx];
	}
	for (int i = 0; i < nx; i++) {
		for (int j = 0; j < nx; j++) {
			sw[i][j].uh = 0;
			sw[i][j].position = glm::vec3(-4.0f + i * dx, 1.0f, -4.0f + j * dx);
			sw[i][j].color = glm::vec3(0.50f, 0.77f, 1.0f);
			sw[i][j].normal = glm::vec3(0.0f, 1.0f, 0.0f);
		}
	}
	sw[6][6].position.y = 1.5f;
	bufferID waterBufferID;
	glGenVertexArrays(1, &waterBufferID.vao);
	glGenBuffers(1, &waterBufferID.bufferID_vertex);
	glGenBuffers(1, &waterBufferID.bufferID_indices);

	// Generate circular obstacle
	circle.center = glm::vec3(0, 0, 0);
	circle.radius = 1.5f;

	// Randomly sample configurations
	vector<glm::vec3> sampleNodes;
	for (int i = 0; i < sampleNumber; i++) {
		GLfloat tempx, tempz;
		tempx = (rand() % 800) / 100.0f - 4.0f;
		tempz = (rand() % 800) / 100.0f - 4.0f;
		if (!circle.pointInside(glm::vec3(tempx, 0, tempz))) {
			sampleNodes.push_back(glm::vec3(tempx, 0, tempz));
		}
	}
	sampleNodes.push_back(startingPoint);
	sampleNodes.push_back(terminalPoint);

	// Generate adjacency matrix
	GLfloat **adjMatrix = new GLfloat*[sampleNodes.size()];
	for (int i = 0; i < sampleNodes.size(); i++) {
		adjMatrix[i] = new GLfloat[sampleNodes.size()];
	}
	for (int i = 0; i < sampleNodes.size(); i++) {
		for (int j = 0; j < sampleNodes.size(); j++) {
			if (i != j) {
				if (circle.lineIntersection(sampleNodes[i], sampleNodes[j])) {
					adjMatrix[i][j] = 9999.0f;
				}
				else {
					adjMatrix[i][j] = sqrtf(glm::dot(sampleNodes[i] - sampleNodes[j], sampleNodes[i] - sampleNodes[j]));
				}
			}
			else {
				adjMatrix[i][j] = 9999.0f;
			}
			//cout << adjMatrix[i][j] <<"   ";
		}
		//cout << endl;
	}

	// Find the shortestpath
	int *shortestPath = new int[sampleNodes.size()];
	bool *finalPath = new bool[sampleNodes.size()];
	GLfloat *weight = new GLfloat[sampleNodes.size()];
	for (int i = 0; i < sampleNodes.size(); i++) {
		finalPath[i] = FALSE;
		weight[i] = adjMatrix[sampleNodes.size() - 2][i];
		if (weight[i] > 0) {
			shortestPath[i] = sampleNodes.size() - 2;
		}

	}
	finalPath[sampleNodes.size() - 2] = TRUE;
	for (int i = 1; i < sampleNodes.size(); i++) {
		GLfloat Min = 9999.0f;
		int minPosition;
		for (int j = 0; j < sampleNodes.size(); j++) {
			//cout << weight[j] << "  ";
			if (!finalPath[j] && (weight[j] < Min)) {
				Min = weight[j];
				minPosition = j;
			}
		}
		//cout << endl;
		finalPath[minPosition] = TRUE;
		for (int j = 0; j < sampleNodes.size(); j++) {
			if (!finalPath[j] && (adjMatrix[minPosition][j] + Min) < weight[j]) {
				weight[j] = adjMatrix[minPosition][j] + Min;
				shortestPath[j] = minPosition;
			}
		}
	}
	vector<glm::vec3> path2;
	path2.push_back(sampleNodes[sampleNodes.size() - 1]);
	int lastNode = shortestPath[sampleNodes.size() - 1];
	while (lastNode != (sampleNodes.size() - 2)) {
		path2.push_back(sampleNodes[lastNode]);
		lastNode = shortestPath[lastNode];
	}
	path2.push_back(sampleNodes[sampleNodes.size() - 2]);
	// Turn the vector upside down
	vector<glm::vec3> path;
	for (int i = 0; i < path2.size(); i++) {
		path.push_back(path2[path2.size() - 1 - i]);
	}

	bufferID roadBufferID;
	glGenVertexArrays(1, &roadBufferID.vao);
	glGenBuffers(1, &roadBufferID.bufferID_vertex);

	GLint uniView = glGetUniformLocation(texturedShader, "view");
	GLint uniProj = glGetUniformLocation(texturedShader, "proj");

	GLint uniViewCloth = glGetUniformLocation(clothShader, "view");
	GLint uniProjCloth = glGetUniformLocation(clothShader, "proj");

	GLint uniViewPhong = glGetUniformLocation(phongShader, "view");
	GLint uniProjPhong = glGetUniformLocation(phongShader, "proj");

	GLint uniViewParticle = glGetUniformLocation(particleShader, "view");
	GLint uniProjParticle = glGetUniformLocation(particleShader, "proj");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE);

	
	GLuint vao, vbo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	list<particle> fire;

	int nbFrames = 0;
	double lastTime = SDL_GetTicks() / 1000.f;
	//Event Loop (Loop forever processing each event as fast as possible)
	SDL_Event windowEvent;
	bool quit = false;
	double frameLastTime = SDL_GetTicks() / 1000.f;
	while (!quit) {
		while (SDL_PollEvent(&windowEvent)) {  //inspect all events in the queue
			if (windowEvent.type == SDL_QUIT) quit = true;
			//List of keycodes: https://wiki.libsdl.org/SDL_Keycode - You can catch many special keys
			//Scancode referes to a keyboard position, keycode referes to the letter (e.g., EU keyboards)
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)
				quit = true; //Exit event loop
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f) { //If "f" is pressed
				fullscreen = !fullscreen;
				SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0); //Toggle fullscreen 
			}

			//SJG: Use key input to change the state of the object
			//     We can use the ".mod" flag to see if modifiers such as shift are pressed
			if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_UP) { //If "up key" is pressed
				glm::vec3 direction = center - position;
				position.x = position.x + direction.x * 0.01;
				position.z = position.z + direction.z * 0.01;
				center.x = center.x + direction.x * 0.01;
				center.z = center.z + direction.z * 0.01;
			}
			if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_DOWN) { //If "down key" is pressed
				glm::vec3 direction = center - position;
				position.x = position.x - direction.x * 0.01;
				position.z = position.z - direction.z * 0.01;
				center.x = center.x - direction.x * 0.01;
				center.z = center.z - direction.z * 0.01;
			}
			if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_SPACE) { //If "space key" is pressed
				glm::vec3 direction = center - position;
				position.y = position.y + 0.03;
				center.y = center.y + 0.03;
			}
			if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_x) { //If "x key" is pressed
				glm::vec3 direction = center - position;
				position.y = position.y - 0.03;
				center.y = center.y - 0.03;
			}
			if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_LEFT) { //If "LEFT key" is pressed
				glm::vec3 direction = center - position;
				glm::mat4 rot;
				rot = glm::rotate(rot, viewRotate * 3.14f, glm::vec3(0.0f, 1.0f, 0.0f));
				glm::vec4 new_direction = rot * glm::vec4(direction, 1);
				center.x = new_direction.x + position.x;
				center.z = new_direction.z + position.z;
			}
			if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_RIGHT) { //If "RIGHT key" is pressed
				glm::vec3 direction = center - position;
				glm::mat4 rot;
				rot = glm::rotate(rot, viewRotate * -3.14f, glm::vec3(0.0f, 1.0f, 0.0f));
				glm::vec4 new_direction = rot * glm::vec4(direction, 1);
				center.x = new_direction.x + position.x;
				center.z = new_direction.z + position.z;
			}
			if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_n) { //If "n key" is pressed
				if ((windSpeed.x - 0.5f) > 0) {
					windSpeed.x -= 0.5f;
				}
			}
			if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_m) { //If "m key" is pressed
				windSpeed.x += 0.5f;
			}
			if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_o) { //If "o key" is pressed
				if (obstacleAdded == FALSE) {
					stateOnRoute trees;
					trees.position = glm::vec3(0, 0, 1.8f);
					trees.velocity = glm::vec3(0, 0, 0);
					trees.radius = treeRadius / 10000.0f;
					treeState.push_back(trees);
				}
				obstacleAdded = TRUE;
			}
			if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_w) { //If "w key" is pressed
				
				treeState[treeState.size() - 1].position.z -= 0.1f;
			}
			if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_s) { //If "s key" is pressed
				treeState[treeState.size() - 1].position.z += 0.1f;
			}
			if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_a) { //If "a key" is pressed
				treeState[treeState.size() - 1].position.x -= 0.1f;
			}
			if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_d) { //If "d key" is pressed
				treeState[treeState.size() - 1].position.x += 0.1f;
			}
		}

		// Clear the screen to default color
		glClearColor(0.4f, 0.6f, 0.9f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(clothShader);

		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR) {
			cerr << "OpenGL error: " << err << endl;
		}

		//if (!saveOutput) lastTime = SDL_GetTicks() / 1000.f;
		//if (saveOutput) lastTime += .07; //Fix framerate at 14 FPS

		glm::mat4 view = glm::lookAt(
			position,						//Cam Position
			center,						//Look at point
			glm::vec3(0.0f, 1.0f, 0.0f));   //Up
		glUniformMatrix4fv(uniViewCloth, 1, GL_FALSE, glm::value_ptr(view));
		
		glm::mat4 proj = glm::perspective(3.14f / 4, screenWidth / (float)screenHeight, 0.01f, 50.0f); //FOV, aspect, near, far
		glUniformMatrix4fv(uniProjCloth, 1, GL_FALSE, glm::value_ptr(proj));

		float dt;
		dt = SDL_GetTicks() / 1000.f - lastTime;
		//cout << dt <<endl;
		lastTime = SDL_GetTicks() / 1000.f;
		
		double currentTime = SDL_GetTicks() / 1000.f;
		nbFrames++;
		if (currentTime - frameLastTime >= 1.0) { 
			cout <<"   FPS: "<<nbFrames<< endl;
			nbFrames = 0;
			frameLastTime += 1.0;
			/*if (length > 1) {
				length--;
			}*/
		}

		/*clo = manageCloth(clo, dt, length, width, footballPosition, footballRadius, windSpeed);
		vector<glm::vec3> fireBirthplace;
		fireBirthplace = sendClothToBuffer(clo, clothShader, length, width, clothBufferID, burntRadius, burntCenter);
		displayCloth(clothShader, clothBufferID, length, width);*/
		
		glUseProgram(particleShader);
		glUniformMatrix4fv(uniViewParticle, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(uniProjParticle, 1, GL_FALSE, glm::value_ptr(proj));

		for (int i = 0; i < queenState.size(); i++) {
			queenState[i].PRM.insert(queenState[i].PRM.begin(), queenState[i].position);
			sendRoadToBuffer(queenState[i].PRM, roadBufferID, particleShader);
		}
		
		//Fire on cloth.
		/*burntRadius += 2.0f * dt;
		fire = manageFireOnCloth(fire, generationRate, dt, fireBirthplace, fireColor, fireLifespan);
		sendDataToBuffer(fire, vao, vbo, particleShader);
		glPointSize(7.0f);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glDrawArrays(GL_POINTS, 0, fire.size());
		glBindVertexArray(0);*/
		/*glUseProgram(phongShader);
		glUniformMatrix4fv(uniViewPhong, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(uniProjPhong, 1, GL_FALSE, glm::value_ptr(proj));
		sw = shallowWater(sw, dt, dx, nx);
		sendWaterToBuffer(sw, nx, waterBufferID, particleShader);
		displayWater(nx, waterBufferID, particleShader);*/

		glUseProgram(texturedShader);
		glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
		queenState = RVO(queenState, dt, treeState, texturedShader);
		renderCircleBeneathModel(queenState, treeState, texturedShader, circleIndex, circleBeneathModelBuffer);
		//cout << pikachuState[0].velocity.x << ' ' << pikachuState[0].velocity.z<<endl;
		drawGeometry(texturedShader, dt, path, path2);
		
		
		SDL_GL_SwapWindow(window); //Double buffering
	}
	glDeleteProgram(clothShader);
	glDeleteProgram(texturedShader);
	//Clean Up

	SDL_GL_DeleteContext(context);
	SDL_Quit();
	return 0;
}

void drawGeometry(int shaderProgram, GLfloat dt, vector<glm::vec3> path, vector<glm::vec3> path2) {

	GLint cam = glGetUniformLocation(shaderProgram, "cam_position");
	glUniform3fv(cam, 1, glm::value_ptr(position));

	// central model
	
	/*footballVelocity += glm::vec3(0, -9.8f, 0) * dt;
	footballPosition += footballVelocity * dt;
	if (footballPosition.y < 0.25f) {
		footballPosition.y = 0.25;
		footballVelocity *= -0.8f;
	}*/
	
	for (int i = 0; i < treeState.size(); i++) {
		glm::mat4 transform2;
		//transform3 = glm::rotate(transform3, lastTime * 3.14f, glm::vec3(0.0, 1.0, 0.0));
		transform2 = glm::translate(transform2, treeState[i].position);
		transform2 = glm::scale(transform2, glm::vec3(0.0001f, 0.0001f, 0.0001f));
		display(shaderProgram, tree, treeBuffer, transform2);
	}

	//glm::mat4 transform3;
	////transform3 = glm::rotate(transform3, lastTime * 3.14f, glm::vec3(0.0, 1.0, 0.0));
	//transform3 = glm::translate(transform3, footballPosition);
	//transform3 = glm::scale(transform3, glm::vec3(circle.radius / footballRadius, circle.radius / footballRadius, circle.radius / footballRadius));
	////display(shaderProgram, football, footballBuffer, transform3);

	glm::mat4 transform4;
	transform4 = glm::translate(transform4, floorPosition);
	transform4 = glm::scale(transform4, glm::vec3(cSpaceWidth / floorWidth, cSpaceWidth / floorWidth, cSpaceWidth / floorWidth));
	display(shaderProgram, Floor, floorBuffer, transform4);

	for (int i = 0; i < queenState.size(); i++) {
		if (sqrtf(glm::dot(queenState[i].goal - queenState[i].position, queenState[i].goal - queenState[i].position)) < 0.001f) {
			queenState[i].position = queenState[i].goal;
			queenState[i].velocity = glm::vec3(0, 0, 0);
		}
		else {
			queenState[i].position += dt * queenState[i].velocity;
		}
		
		//pikachuState = computePositionOnRoute(path, pikachuState);
		GLfloat relativeAngle;
		relativeAngle = acosf(glm::dot(glm::vec3(0, 0, 1), glm::normalize(queenState[i].velocity)));
		if (queenState[i].velocity.x < 0) {
			relativeAngle = 6.28f - relativeAngle;
		}
		glm::mat4 transform5;
		transform5 = glm::translate(transform5, queenState[i].position);
		if (sqrtf(glm::dot(queenState[i].goal - queenState[i].position, queenState[i].goal - queenState[i].position)) > 0.001f) {
			transform5 = glm::rotate(transform5, relativeAngle, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		transform5 = glm::scale(transform5, glm::vec3(0.2f, 0.2f, 0.2f));
		display(shaderProgram, queen, queenBuffer, transform5);
	}

	/*boids = boidMovement(boids, dt, boidRouteRadius, boidNumber, separationRadius, alignmentRadius, cohesionRadius);
	renderBoids(boids, boidsTriangle, boidNumber, shaderProgram, boidsIndex);*/
	/*for (int i = 0; i < boidNumber; i++) {
		glm::mat4 transform7;
		transform7 = glm::translate(transform7, boids[i].position);
		transform7 = glm::scale(transform7, glm::vec3(circle.radius / footballRadius / 10.0f, circle.radius / footballRadius / 10.0f, circle.radius / footballRadius / 10.0f));
		display(shaderProgram, boidModel[i], boidBuffer[i], transform7);
	}*/
}

// Create a NULL-terminated string by reading the provided file
static char* readShaderSource(const char* shaderFile) {
	FILE *fp;
	long length;
	char *buffer;

	// open the file containing the text of the shader code
	fp = fopen(shaderFile, "r");

	// check for errors in opening the file
	if (fp == NULL) {
		printf("can't open shader source file %s\n", shaderFile);
		return NULL;
	}

	// determine the file size
	fseek(fp, 0, SEEK_END); // move position indicator to the end of the file;
	length = ftell(fp);  // return the value of the current position

						 // allocate a buffer with the indicated number of bytes, plus one
	buffer = new char[length + 1];

	// read the appropriate number of bytes from the file
	fseek(fp, 0, SEEK_SET);  // move position indicator to the start of the file
	fread(buffer, 1, length, fp); // read all of the bytes

								  // append a NULL character to indicate the end of the string
	buffer[length] = '\0';

	// close the file
	fclose(fp);

	// return the string
	return buffer;
}

// Create a GLSL program object from vertex and fragment shader files
GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName) {
	GLuint vertex_shader, fragment_shader;
	GLchar *vs_text, *fs_text;
	GLuint program;

	// check GLSL version
	printf("GLSL version: %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	// Create shader handlers
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	// Read source code from shader files
	vs_text = readShaderSource(vShaderFileName);
	fs_text = readShaderSource(fShaderFileName);

	// error check
	if (vs_text == NULL) {
		printf("Failed to read from vertex shader file %s\n", vShaderFileName);
		exit(1);
	}
	else if (DEBUG_ON) {
		printf("Vertex Shader:\n=====================\n");
		printf("%s\n", vs_text);
		printf("=====================\n\n");
	}
	if (fs_text == NULL) {
		printf("Failed to read from fragent shader file %s\n", fShaderFileName);
		exit(1);
	}
	else if (DEBUG_ON) {
		printf("\nFragment Shader:\n=====================\n");
		printf("%s\n", fs_text);
		printf("=====================\n\n");
	}

	// Load Vertex Shader
	const char *vv = vs_text;
	glShaderSource(vertex_shader, 1, &vv, NULL);  //Read source
	glCompileShader(vertex_shader); // Compile shaders

									// Check for errors
	GLint  compiled;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		printf("Vertex shader failed to compile:\n");
		if (DEBUG_ON) {
			GLint logMaxSize, logLength;
			glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
			printf("printing error message of %d bytes\n", logMaxSize);
			char* logMsg = new char[logMaxSize];
			glGetShaderInfoLog(vertex_shader, logMaxSize, &logLength, logMsg);
			printf("%d bytes retrieved\n", logLength);
			printf("error message: %s\n", logMsg);
			delete[] logMsg;
		}
		exit(1);
	}

	// Load Fragment Shader
	const char *ff = fs_text;
	glShaderSource(fragment_shader, 1, &ff, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);

	//Check for Errors
	if (!compiled) {
		printf("Fragment shader failed to compile\n");
		if (DEBUG_ON) {
			GLint logMaxSize, logLength;
			glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
			printf("printing error message of %d bytes\n", logMaxSize);
			char* logMsg = new char[logMaxSize];
			glGetShaderInfoLog(fragment_shader, logMaxSize, &logLength, logMsg);
			printf("%d bytes retrieved\n", logLength);
			printf("error message: %s\n", logMsg);
			delete[] logMsg;
		}
		exit(1);
	}

	// Create the program
	program = glCreateProgram();

	// Attach shaders to program
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	// Link and set program to use
	glLinkProgram(program);

	return program;
}


