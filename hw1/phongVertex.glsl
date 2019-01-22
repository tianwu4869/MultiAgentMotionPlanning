#version 150 core

in vec3 position;
in vec3 inColor;
in vec3 inNormal;

const vec3 inLightDir = normalize(vec3(1,-1,1));

out vec3 Color;
out vec3 normal;
out vec3 pos;
out vec3 lightDir;

uniform mat4 view;
uniform mat4 proj;

void main() {
   Color = inColor;
   gl_Position = proj * view * vec4(position,1.0);
   pos = (view * vec4(position,1.0)).xyz;
   lightDir = (view * vec4(inLightDir,0.0)).xyz; //It's a vector!
   vec4 norm4 = transpose(inverse(view)) * vec4(inNormal,0.0);
   normal = normalize(norm4.xyz);
}