#version 330 core
in vec2      fTexCoords;
flat in uint fTexIndex;

out vec4 color;

uniform sampler2DArray blockTextureArray;

void main()
{
  color = texture(blockTextureArray, vec3(fTexCoords, float(fTexIndex)));
}
