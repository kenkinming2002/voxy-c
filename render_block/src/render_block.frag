#version 460 core

uniform sampler2D left;
uniform sampler2D right;
uniform sampler2D top;

flat in uint v_face_index;
in vec2 v_texture_coords;

out vec4 color;

void main()
{
  switch(v_face_index)
  {
  case 0: color = texture(left, v_texture_coords); break;
  case 1: color = texture(right, v_texture_coords); break;
  case 2: color = texture(top, v_texture_coords); break;
  }
}
