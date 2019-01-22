#version 150 core

in vec3 position;
in vec3 inColor;
in vec2 inTexcoord;

out vec3 Color;
out vec2 texcoord;

uniform mat4 view;
uniform mat4 proj;

void main() {
   Color = inColor;
   gl_Position = proj * view * vec4(position,1.0);
   texcoord = inTexcoord;
}