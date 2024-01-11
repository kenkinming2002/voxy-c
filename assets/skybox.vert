#version 330 core
layout (location = 0) in vec3 vPosition;

out vec3 fTexCoords;

uniform mat4 RP;

void main()
{
  fTexCoords.x =  vPosition.x;
  fTexCoords.y =  vPosition.z;
  fTexCoords.z = -vPosition.y;
  gl_Position = RP * vec4(vPosition, 1.0);
}
