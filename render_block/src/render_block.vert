#version 460 core

uniform mat4 VP;

flat out uint v_face_index;
out vec2 v_texture_coords;

uint get_face_index()
{
  return gl_VertexID / 6;
}

uint get_vertex_index()
{
  switch(gl_VertexID % 6)
  {
  case 0: return 0;
  case 1: return 1;
  case 2: return 2;
  case 3: return 2;
  case 4: return 1;
  case 5: return 3;
  }
}

bool is_face_up()
{
  switch(get_face_index())
  {
  case 0: return false;
  case 1: return false;
  case 2: return true;
  }
}

vec3 get_face_normal()
{
  switch(get_face_index())
  {
  case 0: return vec3(1.0f, 0.0f, 0.0f);
  case 1: return vec3(0.0f, 1.0f, 0.0f);
  case 2: return vec3(0.0f, 0.0f, 1.0f);
  }
}

vec3 get_position()
{
  vec3 normal = get_face_normal();
  vec3 axis2 = is_face_up() ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 0.0, 1.0);
  vec3 axis1 = cross(normal, axis2);

  switch(get_vertex_index())
  {
  case 0: return 0.5 * normal + -0.5 * axis1 + -0.5 * axis2;
  case 1: return 0.5 * normal + +0.5 * axis1 + -0.5 * axis2;
  case 2: return 0.5 * normal + -0.5 * axis1 + +0.5 * axis2;
  case 3: return 0.5 * normal + +0.5 * axis1 + +0.5 * axis2;
  }
}

vec2 get_texture_coords()
{
  switch(get_vertex_index())
  {
  case 0: return vec2(0.0, 0.0);
  case 1: return vec2(1.0, 0.0);
  case 2: return vec2(0.0, 1.0);
  case 3: return vec2(1.0, 1.0);
  }
}

void main()
{
  gl_Position = VP * vec4(get_position(), 1.0);
  v_face_index = get_face_index();
  v_texture_coords = get_texture_coords();
}

