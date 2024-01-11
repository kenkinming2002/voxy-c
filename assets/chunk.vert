#version 330 core
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vColor;

out vec4 fColor;

uniform mat4 MVP;

void main()
{
  gl_Position = MVP * vec4(vPosition, 1.0);
  fColor      = vec4(vColor, 1.0);
}

