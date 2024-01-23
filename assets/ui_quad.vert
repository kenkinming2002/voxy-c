#version 330 core

uniform vec2 window_size;
uniform vec2 position;
uniform vec2 dimension;

void main()
{
  const vec2[6] factors = vec2[6](vec2(0.0, 0.0),
                                  vec2(1.0, 0.0),
                                  vec2(0.0, 1.0),
                                  vec2(0.0, 1.0),
                                  vec2(1.0, 0.0),
                                  vec2(1.0, 1.0));

  vec2 ndc_position  = (position  / window_size) * 2.0 - 1.0;
  vec2 ndc_dimension = (dimension / window_size) * 2.0;

  gl_Position = vec4(ndc_position + ndc_dimension * factors[gl_VertexID], 0.0, 1.0);
}
