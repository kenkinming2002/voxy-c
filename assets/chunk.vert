#version 330 core
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoords;
layout(location = 2) in uint vTexIndex;

out vec2      fTexCoords;
flat out uint fTexIndex;

out float visibility;

uniform mat4 VP;
uniform mat4 V;

const float fogDensity  = 0.01;
const float fogGradient = 1.0;

void main()
{
  gl_Position = VP * vec4(vPosition, 1.0);

  fTexCoords = vTexCoords;
  fTexIndex  = vTexIndex;

  vec4  pos  = V * vec4(vPosition, 1.0);
  float dist = length(pos.xyz);

  visibility = clamp(exp(-pow(dist * fogDensity, fogGradient)), 0.0, 1.0);
}

