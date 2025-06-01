#version 460 core
layout(location = 0) in uint  v_center;
layout(location = 1) in uint  v_metadata1;
layout(location = 2) in uint  v_metadata2;
layout(location = 3) in float v_damage;

out vec2      f_texture_coords;
flat out uint f_texture_index;

out mat2 f_luminances;

out float f_visibility;
out float f_damage;

uniform mat4 MVP;
uniform mat4 MV;

const float fogDensity  = 0.01;
const float fogGradient = 1.0;

vec3 get_normal(uint normal_index)
{
  switch(normal_index)
  {
  case 0u: return vec3(-1.0, 0.0, 0.0);
  case 1u: return vec3( 1.0, 0.0, 0.0);
  case 2u: return vec3(0.0, -1.0, 0.0);
  case 3u: return vec3(0.0,  1.0, 0.0);
  case 4u: return vec3(0.0, 0.0, -1.0);
  case 5u: return vec3(0.0, 0.0,  1.0);
  }
}

bool is_up(uint normal_index)
{
  switch(normal_index)
  {
  case 0u: return false;
  case 1u: return false;
  case 2u: return false;
  case 3u: return false;
  case 4u: return true;
  case 5u: return true;
  }
}

vec2 get_texture_coords()
{
  switch(gl_VertexID)
  {
  case 0: return vec2(0.0, 0.0);
  case 1: return vec2(1.0, 0.0);
  case 2: return vec2(0.0, 1.0);
  case 3: return vec2(1.0, 1.0);
  }
}

void main()
{
  ivec3 center = ivec3(
      bitfieldExtract(v_center, 0, 4),
      bitfieldExtract(v_center, 4, 4),
      bitfieldExtract(v_center, 8, 4));

  uint normal_index = bitfieldExtract(v_metadata2, 16, 3);
  uint texture_index = bitfieldExtract(v_metadata2, 19, 13);

  vec3 normal = get_normal(normal_index);

  vec3 axis2 = is_up(normal_index) ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 0.0, 1.0);
  vec3 axis1 = cross(normal, axis2);

  vec2 texture_coords = get_texture_coords();

  vec3 dir1 = axis1 * (texture_coords.x * 2.0 - 1.0);
  vec3 dir2 = axis2 * (texture_coords.y * 2.0 - 1.0);

  vec3 position = vec3(center) + 0.5 * (normal + dir1 + dir2);

  gl_Position = MVP * vec4(position, 1.0);

  f_texture_coords = texture_coords;
  f_texture_index  = texture_index;

  for(int v = 0; v < 2; ++v)
    for(int u = 0; u < 2; ++u)
    {
      uint light_level = bitfieldExtract(v_metadata1, 8 * (2 * v + u),  8);
      uint count       = bitfieldExtract(v_metadata2, 4 * (2 * v + u),  4);

      float luminance = float(light_level) / float(count) / 15.0 * mix(0.3, 1.0, float(count) / 8.0);
      f_luminances[v][u] = luminance;
    }


  vec4  view_position = MV * vec4(position, 1.0);
  float view_distance = length(view_position.xyz);
  f_visibility = clamp(exp(-pow(view_distance * fogDensity, fogGradient)), 0.0, 1.0);

  f_damage = v_damage;
}

