#version 460 core
uniform mat4 VP;

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;

layout(location = 3) in mat4 v_transform;
layout(location = 7) in float v_light;

out vec2 f_uv;
out float f_light;

void main()
{
  gl_Position = VP * transpose(v_transform) * vec4(v_position, 1.0);

  f_uv = v_uv;
  f_light = v_light;
}


