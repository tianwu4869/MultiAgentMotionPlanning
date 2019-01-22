#version 150 core

in vec3 Color;
in vec2 texcoord;

out vec4 outColor;

uniform sampler2D texID;

void main() {
  
  outColor = vec4(texture(texID, texcoord).bgr, 1);

}