#version 460 core
in vec2      f_texture_coords;
flat in uint f_texture_index;
in float     f_light;
in float     f_visibility;
in float     f_damage;

out vec4 color;

uniform sampler2DArray blockTextureArray;

const vec4 skyColor = vec4(0.2, 0.3, 0.3, 1.0);

float rand(vec2 position)
{
  return fract(sin(dot(position, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
  vec4 baseColor = texture(blockTextureArray, vec3(f_texture_coords, float(f_texture_index)));

  // Light
  {
    float factor = mix(f_light, 1.0, 0.01);
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

