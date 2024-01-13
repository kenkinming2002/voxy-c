#version 330 core
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoords;
layout(location = 2) in uint vTexIndex;

out vec2      fTexCoords;
flat out uint fTexIndex;

uniform mat4 VP;

void main()
{
  gl_Position = VP * vec4(vPosition, 1.0);

  fTexCoords = vTexCoords;
  fTexIndex  = vTexIndex;
}

