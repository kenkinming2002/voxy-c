#version 330 core
in vec2      fTexCoords;
flat in uint fTexIndex;
in float     fLightLevel;

in float visibility;

out vec4 color;

uniform sampler2DArray blockTextureArray;

const vec4 skyColor = vec4(0.2, 0.3, 0.3, 1.0);

void main()
{
  vec4 tmp = texture(blockTextureArray, vec3(fTexCoords, float(fTexIndex)));
  tmp = tmp * vec4(fLightLevel, fLightLevel, fLightLevel, 1.0);
  tmp = mix(skyColor, tmp, visibility);
  color = tmp;
}

