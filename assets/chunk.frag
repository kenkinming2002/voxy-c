#version 330 core
in vec2      fTexCoords;
flat in uint fTexIndex;

in float visibility;

out vec4 color;

uniform sampler2DArray blockTextureArray;

const vec3 skyColor = vec3(0.2, 0.3, 0.3);

void main()
{
  color = vec4(mix(skyColor, texture(blockTextureArray, vec3(fTexCoords, float(fTexIndex))).rgb, visibility), 1.0);
}

