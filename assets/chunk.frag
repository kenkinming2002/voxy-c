#version 330 core
in vec2      fTexCoords;
flat in uint fTexIndex;
in float     fLightLevel;

in float visibility;

out vec4 color;

uniform sampler2DArray blockTextureArray;

const vec3 skyColor = vec3(0.2, 0.3, 0.3);

void main()
{
  vec3 tmp = texture(blockTextureArray, vec3(fTexCoords, float(fTexIndex))).rgb;
  tmp = tmp * fLightLevel;
  tmp = mix(skyColor, tmp, visibility);
  color = vec4(tmp, 1.0);
}

