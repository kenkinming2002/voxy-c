#version 460 core
in vec2      f_texture_coords;
flat in uint f_texture_index;
in float     f_light;
in float     f_visibility;

out vec4 color;

uniform sampler2DArray blockTextureArray;

const vec4 skyColor = vec4(0.2, 0.3, 0.3, 1.0);

void main()
{
  vec4 tmp = texture(blockTextureArray, vec3(f_texture_coords, float(f_texture_index)));
  float lightLevel = mix(f_light, 1.0, 0.3);
  tmp = tmp * vec4(lightLevel, lightLevel, lightLevel, 1.0);
  tmp = mix(skyColor, tmp, f_visibility);
  color = tmp;
}

