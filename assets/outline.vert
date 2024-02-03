#version 330 core

uniform mat4 VP;
uniform vec3 position;

void main()
{
  const vec3[24] factors = vec3[24](
      vec3(0.0, 0.0, 0.0), vec3(1.0, 0.0, 0.0),
      vec3(0.0, 1.0, 0.0), vec3(1.0, 1.0, 0.0),
      vec3(0.0, 0.0, 1.0), vec3(1.0, 0.0, 1.0),
      vec3(0.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0),

      vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0),
      vec3(1.0, 0.0, 0.0), vec3(1.0, 1.0, 0.0),
      vec3(0.0, 0.0, 1.0), vec3(0.0, 1.0, 1.0),
      vec3(1.0, 0.0, 1.0), vec3(1.0, 1.0, 1.0),

      vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0),
      vec3(1.0, 0.0, 0.0), vec3(1.0, 0.0, 1.0),
      vec3(0.0, 1.0, 0.0), vec3(0.0, 1.0, 1.0),
      vec3(1.0, 1.0, 0.0), vec3(1.0, 1.0, 1.0));

  gl_Position = VP * vec4(position + factors[gl_VertexID], 1.0);
}


