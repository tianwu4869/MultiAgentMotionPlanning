//CSCI 5611 OpenGL Animation Tutorial 
//A 1D bouncing ball

//Running on Mac OSX
//  Download the SDL2 Framework from here: https://www.libsdl.org/download-2.0.php
//  Open the .dmg and move the file SDL2.Framework into the directory /Library/Frameworks/
//  Make sure you place this cpp file in the same directory with the "glad" folder and the "glm" folder
//  g++ Bounce.cpp glad/glad.c -framework OpenGL -framework SDL2; ./a.out

//Running on Windows
//  Download the SDL2 *Development Libararies* from here: https://www.libsdl.org/download-2.0.php
//  Place SDL2.dll, the 3 .lib files, and the include directory in locations known to MSVC
//  Add both Bounce.cpp and glad/glad.c to the project file
//  Compile and run

//Running on Ubuntu
//  sudo apt-get install libsdl2-2.0-0 libsdl2-dev
//  Make sure you place this cpp file in the same directory with the "glad" folder and the "glm" folder
//  g++ Bounce.cpp glad/glad.c -lGL -lSDL; ./a.out

#include "stdafx.h"
#include "glad/glad.h"  //Include order can matter here
#ifndef _WIN32
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif
#include <cstdio>

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <cstdio>
using namespace std;

bool saveOutput = false; //Make to true to save out your animation
int screen_width = 800;
int screen_height = 600;


// Shader sources
const GLchar* vertexSource =
"#version 150 core\n"
"in vec3 position;"
"in vec3 inColor;"
"in vec3 inNormal;"
"const vec3 inLightDir = normalize(vec3(0,2,2));"
"out vec3 Color;"
"out vec3 normal;"
"out vec3 lightDir;"
"uniform mat4 model;"
"uniform mat4 view;"
"uniform mat4 proj;"
"void main() {"
"   Color = inColor;"
"   gl_Position = proj * view * model * vec4(position, 1.0);"
"   vec4 norm4 = transpose(inverse(model)) * vec4(inNormal, 1.0);"
"   normal = normalize(norm4.xyz);"
"   lightDir = (view * vec4(inLightDir, 0)).xyz;"
"}";

const GLchar* fragmentSource =
"#version 150 core\n"
"in vec3 Color;"
"in vec3 normal;"
"in vec3 lightDir;"
"out vec4 outColor;"
"const float ambient = .2;"
"void main() {"
"   vec3 diffuseC = Color * max(dot(lightDir, normal), 0);"
"   vec3 ambC = Color * ambient;"
"   outColor = vec4(diffuseC+ambC, 1.0);"
"}";

bool fullscreen = false;
void Win2PPM(int width, int height);
void draw(float dt);

//Index of where to model, view, and projection matricies are stored on the GPU
GLint uniModel, uniView, uniProj;

float aspect; //aspect ratio (needs to be updated if the window is resized)

int main(int argc, char *argv[]) {
	SDL_Init(SDL_INIT_VIDEO);  //Initialize Graphics (for OpenGL)

	//Ask SDL to get a recent version of OpenGL (3.2 or greater)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	//Create a window (offsetx, offsety, width, height, flags)
	SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 100, 100, screen_width, screen_height, SDL_WINDOW_OPENGL);
	aspect = screen_width / (float)screen_height; //aspect ratio (needs to be updated if the window is resized)

	//The above window cannot be resized which makes some code slightly easier.
	//Below show how to make a full screen window or allow resizing
	//SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 0, 0, screen_width, screen_height, SDL_WINDOW_FULLSCREEN|SDL_WINDOW_OPENGL);
	//SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 100, 100, screen_width, screen_height, SDL_WINDOW_RESIZABLE|SDL_WINDOW_OPENGL);
	//SDL_Window* window = SDL_CreateWindow("My OpenGL Program",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,0,0,SDL_WINDOW_FULLSCREEN_DESKTOP|SDL_WINDOW_OPENGL); //Boarderless window "fake" full screen



	//Create a context to draw in
	SDL_GLContext context = SDL_GL_CreateContext(window);

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

	//Build a Vertex Array Object. This stores the VBO and attribute mappings in one object
	GLuint vao;
	glGenVertexArrays(1, &vao); //Create a VAO
	glBindVertexArray(vao); //Bind the above created VAO to the current context

	float vertices[] = {
		// X      Y     Z     R     G      B      U      V
		  -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		   0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		   0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		   0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		  -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		  -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,

		  -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		   0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		   0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		   0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		  -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		  -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,

		  -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		  -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		  -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		  -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		  -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		  -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

		   0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		   0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		   0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		   0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		   0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		   0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,

		  -0.5f, -0.5f, -0.5f, 0.2f, 0.2f, 0.2f, 0.0f, 1.0f,
		   0.5f, -0.5f, -0.5f, 0.2f, 0.2f, 0.2f, 1.0f, 1.0f,
		   0.5f, -0.5f,  0.5f, 0.2f, 0.2f, 0.2f, 1.0f, 0.0f,
		   0.5f, -0.5f,  0.5f, 0.2f, 0.2f, 0.2f, 1.0f, 0.0f,
		  -0.5f, -0.5f,  0.5f, 0.2f, 0.2f, 0.2f, 0.0f, 0.0f,
		  -0.5f, -0.5f, -0.5f, 0.2f, 0.2f, 0.2f, 0.0f, 1.0f,

		  -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
		   0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		   0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
		   0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
		  -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
		  -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f
	};

	float normals[] = {
		0.f,0.f,-1.f,0.f,0.f,-1.f,0.f,0.f,-1.f,0.f,0.f,-1.f,0.f,0.f,-1.f,0.f,0.f,-1.f,
		0.f,0.f,1.f,0.f,0.f,1.f,0.f,0.f,1.f,0.f,0.f,1.f,0.f,0.f,1.f,0.f,0.f,1.f,
		-1.f,0.f,0.f,-1.f,0.f,0.f,-1.f,0.f,0.f,-1.f,0.f,0.f,-1.f,0.f,0.f,-1.f,0.f,0.f,
		1.f,0.f,0.f,1.f,0.f,0.f,1.f,0.f,0.f,1.f,0.f,0.f,1.f,0.f,0.f,1.f,0.f,0.f,
		0.f,-1.f,0.f,0.f,-1.f,0.f,0.f,-1.f,0.f,0.f,-1.f,0.f,0.f,-1.f,0.f,0.f,-1.f,0.f,
		0.f,1.f,0.f,0.f,1.f,0.f,0.f,1.f,0.f,0.f,1.f,0.f,0.f,1.f,0.f,0.f,1.f,0.f,
	};

	//Allocate memory on the graphics card to store geometry (vertex buffer object)
	GLuint vbo[2];
	glGenBuffers(2, vbo);  //Create 1 buffer called vbo
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); //Set the vbo as the active array buffer (Only one buffer can be active at a time)
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //upload vertices to vbo
	//GL_STATIC_DRAW means we won't change the geometry, GL_DYNAMIC_DRAW = geometry changes infrequently
	//GL_STREAM_DRAW = geom. changes frequently.  This effects which types of GPU memory is used


	//Load the vertex Shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	//Let's double check the shader compiled 
	GLint status;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
	if (!status) {
		char buffer[512];
		glGetShaderInfoLog(vertexShader, 512, NULL, buffer);
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
			"Compilation Error",
			"Failed to Compile: Check Consol Output.",
			NULL);
		printf("Vertex Shader Compile Failed. Info:\n\n%s\n", buffer);
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	//Double check the shader compiled 
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
	if (!status) {
		char buffer[512];
		glGetShaderInfoLog(fragmentShader, 512, NULL, buffer);
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
			"Compilation Error",
			"Failed to Compile: Check Consol Output.",
			NULL);
		printf("Fragment Shader Compile Failed. Info:\n\n%s\n", buffer);
	}

	//Join the vertex and fragment shaders together into one program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor"); // set output
	glLinkProgram(shaderProgram); //run the linker

	glUseProgram(shaderProgram); //Set the active shader (only one can be used at a time)


	//Tell OpenGL how to set fragment shader input 
	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
	//Attribute, vals/attrib., type, normalized?, stride, offset
	//Binds to VBO current GL_ARRAY_BUFFER 
	glEnableVertexAttribArray(posAttrib);

	GLint colAttrib = glGetAttribLocation(shaderProgram, "inColor");
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(colAttrib);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW); //upload normals to vbo
	GLint normAttrib = glGetAttribLocation(shaderProgram, "inNormal");
	glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(normAttrib);

	//Where to model, view, and projection matricies are stored on the GPU
	uniModel = glGetUniformLocation(shaderProgram, "model");
	uniView = glGetUniformLocation(shaderProgram, "view");
	uniProj = glGetUniformLocation(shaderProgram, "proj");

	glEnable(GL_DEPTH_TEST);

	//Event Loop (Loop forever processing each event as fast as possible)
	SDL_Event windowEvent;
	bool quit = false;
	float lastTime = SDL_GetTicks() / 1000.f;
	float dt = 0;
	while (!quit) {
		while (SDL_PollEvent(&windowEvent)) {
			if (windowEvent.type == SDL_QUIT) quit = true; //Exit event loop
		  //List of keycodes: https://wiki.libsdl.org/SDL_Keycode - You can catch many special keys
		  //Scancode referes to a keyboard position, keycode referes to the letter (e.g., EU keyboards)
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)
				quit = true; ; //Exit event loop
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f) //If "f" is pressed
				fullscreen = !fullscreen;
			SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0); //Set to full screen 
		}

		// Clear the screen to default color
		glClearColor(.2f, 0.4f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (!saveOutput) dt = (SDL_GetTicks() / 1000.f) - lastTime;
		if (dt > .1) dt = .1; //Have some max dt
		lastTime = SDL_GetTicks() / 1000.f;
		if (saveOutput) dt += .07; //Fix framerate at 14 FPS

		draw(dt);

		if (saveOutput) Win2PPM(screen_width, screen_height);


		SDL_GL_SwapWindow(window); //Double buffering
	}

	//Clean Up
	glDeleteProgram(shaderProgram);
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);

	glDeleteBuffers(1, vbo);

	glDeleteVertexArrays(1, &vao);

	SDL_GL_DeleteContext(context);
	SDL_Quit();
	return 0;
}

void Win2PPM(int width, int height) {
	char outdir[10] = "out/"; //Must be defined!
	int i, j;
	FILE* fptr;
	static int counter = 0;
	char fname[32];
	unsigned char *image;

	/* Allocate our buffer for the image */
	image = (unsigned char *)malloc(3 * width*height*sizeof(char));
	if (image == NULL) {
		fprintf(stderr, "ERROR: Failed to allocate memory for image\n");
	}

	/* Open the file */
	sprintf(fname, "%simage_%04d.ppm", outdir, counter);
	if ((fptr = fopen(fname, "w")) == NULL) {
		fprintf(stderr, "ERROR: Failed to open file for window capture\n");
	}

	/* Copy the image into our buffer */
	glReadBuffer(GL_BACK);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);

	/* Write the PPM file */
	fprintf(fptr, "P6\n%d %d\n255\n", width, height);
	for (j = height - 1; j >= 0; j--) {
		for (i = 0; i < width; i++) {
			fputc(image[3 * j*width + 3 * i + 0], fptr);
			fputc(image[3 * j*width + 3 * i + 1], fptr);
			fputc(image[3 * j*width + 3 * i + 2], fptr);
		}
	}

	free(image);
	fclose(fptr);
	counter++;
}

float position = 0; float velocity = 5;
float radius = 2; float floorPos = -10;

void computePhysics(float dt) {
	float acceleration = -10;//-9.8;
	velocity = velocity + acceleration * dt;
	position = position + velocity * dt;
	printf("pos: %.2f, vel: %.2f\n", position, velocity);
	if (position - radius < floorPos) {
		position = floorPos + radius;
		velocity *= -.95;
	}
}

void draw(float dt) {
	//glm normally initializes matrices to 0, so you must give it the diagonal value.
	glm::mat4 model(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, position));
	model = glm::scale(model, glm::vec3(radius));
	glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

	computePhysics(dt);


	//Set the Camera view paramters (FVO, aspect ratio, etc.)
	glm::mat4 proj = glm::perspective(3.14f / 4, aspect, .1f, 100.0f); //FOV, aspect, near, far
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

	//Set the Camera Position and Orientation
	glm::mat4 view = glm::lookAt(
		glm::vec3(20.f, 20.f, 1.8f),  //Cam Position
		glm::vec3(0.0f, 0.0f, 0.0f),  //Look at point
		glm::vec3(0.0f, 0.0f, 1.0f)); //Up
	glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));


	glDrawArrays(GL_TRIANGLES, 0, 36); //(Primitives, Which VBO, Number of vertices)
}
