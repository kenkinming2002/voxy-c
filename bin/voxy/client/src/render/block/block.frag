#version 460 core

in vec2      f_texture_coords;
flat in uint f_texture_index;

flat in mat2 f_luminances;

in float f_visibility;
in float f_damage;

out vec4 color;

uniform sampler2DArray blockTextureArray;

const vec4 skyColor = vec4(0.2, 0.3, 0.3, 1.0);

float rand(vec2 position)
{
  return fract(sin(dot(position, vec2(12.9898, 78.233))) * 43758.5453);
}

float interpolate(mat2 value)
{
  return mix(mix(value[0][0], value[0][1], f_texture_coords.x),
             mix(value[1][0], value[1][1], f_texture_coords.x),
             f_texture_coords.y);
}

void main()
{
  vec4 baseColor = texture(blockTextureArray, vec3(f_texture_coords, float(f_texture_index)));

  // Luminance
  {
    float factor = mix(interpolate(f_luminances), 1.0, 0.1);
    baseColor = baseColor * vec4(factor, factor, factor, 1.0);
  }

  // Damage
  {
    vec2 position = floor(f_texture_coords * 16.0) * 16.0;
    float factor = 1.0 - floor(rand(position) * f_damage / 0.2) * 0.15;
    baseColor = baseColor * vec4(factor, factor, factor, 1.0);
  }

  color = mix(skyColor, baseColor, f_visibility);
}

