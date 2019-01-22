#version 150 core

in vec3 position;
in vec3 inColor;

//const vec3 inColor = vec3(0.f,0.7f,0.f);
//const vec3 inLightDir = normalize(vec3(1,-1,1));

out vec3 Color;

uniform mat4 view;
uniform mat4 proj;

void main() {
   Color = inColor;
   gl_Position = proj * view * vec4(position,1.0);
}