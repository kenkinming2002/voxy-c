#version 460 core
layout(location = 0) in ivec3 v_center;
layout(location = 1) in uint  v_normal_index_and_texture_index;
layout(location = 2) in uint  v_light_level_and_occlusion_counts;

out vec2      f_texture_coords;
flat out uint f_texture_index;
out float     f_light;
out float     f_visibility;

uniform mat4 VP;
uniform mat4 V;

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

uint get_light_level()
{
  return bitfieldExtract(v_light_level_and_occlusion_counts, 0, 4);
}

uint get_occlusion_count()
{
  switch(gl_VertexID)
  {
  case 0: return bitfieldExtract(v_light_level_and_occlusion_counts, 4,  4);
  case 1: return bitfieldExtract(v_light_level_and_occlusion_counts, 8,  4);
  case 2: return bitfieldExtract(v_light_level_and_occlusion_counts, 12, 4);
  case 3: return bitfieldExtract(v_light_level_and_occlusion_counts, 16, 4);
  }
}

void main()
{
  uint normal_index  = bitfieldExtract(v_normal_index_and_texture_index, 0, 3);
  uint texture_index = bitfieldExtract(v_normal_index_and_texture_index, 3, 29);

  vec3 normal = get_normal(normal_index);

  vec3 axis2 = is_up(normal_index) ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 0.0, 1.0);
  vec3 axis1 = cross(normal, axis2);

  vec2 texture_coords = get_texture_coords();

  vec3 dir1 = axis1 * (texture_coords.x * 2.0 - 1.0);
  vec3 dir2 = axis2 * (texture_coords.y * 2.0 - 1.0);

  vec3 position = vec3(v_center) + 0.5 * (normal + dir1 + dir2);

  gl_Position = VP * vec4(position, 1.0);

  f_texture_coords = texture_coords;
  f_texture_index  = texture_index;

  uint light_level     = get_light_level();
  uint occlusion_count = get_occlusion_count();
  f_light = (float(light_level) / 15.0 * 0.9 + 0.1) * (1.0 - float(occlusion_count) / float(2 * 2 * 2));

  vec4  view_position = V * vec4(position, 1.0);
  float view_distance = length(view_position.xyz);
  f_visibility = clamp(exp(-pow(view_distance * fogDensity, fogGradient)), 0.0, 1.0);
}

