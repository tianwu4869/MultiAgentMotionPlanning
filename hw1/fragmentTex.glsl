#version 150 core

in vec3 Color;
in vec3 normal;
in vec3 pos;
in vec3 lightDir;
in vec2 texcoord;

out vec4 outColor;

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex[30];

uniform int texID;
uniform int getKey;
uniform vec3 cam_position;
uniform float dr;
uniform float dg;
uniform float db;
uniform float ar;
uniform float ag;
uniform float ab;
uniform float sr;
uniform float sg;
uniform float sb;
uniform float ns;

const float ambient = .3;
void main() {
  vec3 color;
  vec3 whiteLight = vec3(1.0, 1.0, 1.0);
  if (texID == 0){
    color = whiteLight;
  }
  else if(texID == -1){
    color = Color;
  }
  else {
   	color = texture(tex[texID], texcoord).bgr;
  }
  
  vec3 lightDirection = normalize(lightDir);
   float distance = sqrt(pow(cam_position.x - pos.x, 2) + pow(cam_position.y - pos.y, 2) + pow(cam_position.z - pos.z, 2));
   vec3 diffuseC = vec3(color.x * dr, color.y * dg,color.z * db)*max(dot(lightDirection,normal),0.0);
   vec3 ambC = vec3(color.x * ar, color.y * ag,color.z * ab);
   vec3 viewDir = normalize(cam_position-pos); //We know the eye is at (0,0)!
   vec3 h = normalize(viewDir + lightDirection);
   vec3 specC = vec3(color.x * sr, color.y * sg,color.z * sb)*pow(max(0, dot(normal, h)), ns);
   vec3 oColor = ambC + diffuseC + specC;
   if(oColor.x >= 1.0){
     oColor.x == 1.0;
   }
   if(oColor.y >= 1.0){
     oColor.y == 1.0;
   }
   if(oColor.z >= 1.0){
     oColor.z == 1.0;
   }
	   outColor = vec4(oColor ,1);
}