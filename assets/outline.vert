#version 330 core

uniform mat4 VP;
uniform vec3 position;
uniform vec3 dimension;

void main()
{
  const vec3[24] factors = vec3[24](
      vec3(-0.5, -0.5, -0.5), vec3( 0.5, -0.5, -0.5),
      vec3(-0.5,  0.5, -0.5), vec3( 0.5,  0.5, -0.5),
      vec3(-0.5, -0.5,  0.5), vec3( 0.5, -0.5,  0.5),
      vec3(-0.5,  0.5,  0.5), vec3( 0.5,  0.5,  0.5),

      vec3(-0.5, -0.5, -0.5), vec3(-0.5,  0.5, -0.5),
      vec3( 0.5, -0.5, -0.5), vec3( 0.5,  0.5, -0.5),
      vec3(-0.5, -0.5,  0.5), vec3(-0.5,  0.5,  0.5),
      vec3( 0.5, -0.5,  0.5), vec3( 0.5,  0.5,  0.5),

      vec3(-0.5, -0.5, -0.5), vec3(-0.5, -0.5,  0.5),
      vec3( 0.5, -0.5, -0.5), vec3( 0.5, -0.5,  0.5),
      vec3(-0.5,  0.5, -0.5), vec3(-0.5,  0.5,  0.5),
      vec3( 0.5,  0.5, -0.5), vec3( 0.5,  0.5,  0.5));

  gl_Position = VP * vec4(position + factors[gl_VertexID] * dimension, 1.0);
}


